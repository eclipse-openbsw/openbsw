// Copyright 2024 Accenture.

#include "can/transceiver/canflex2/CanFlex2Transceiver.h"

#include <async/Types.h>
#include <can/CanLogger.h>
#include <can/framemgmt/IFilteredCANFrameSentListener.h>
#include <common/busid/BusId.h>
#include <etl/delegate.h>
#include <util/estd/assert.h>

#include <platform/config.h>
#include <platform/estdint.h>

namespace logger = ::util::logger;

namespace bios
{
CanFlex2Transceiver* CanFlex2Transceiver::fpCanTransceivers[8] = {0};

CanFlex2Transceiver::CanFlex2Transceiver(
    ::async::ContextType context,
    uint8_t const busId,
    FlexCANDevice::Config const& devConfig,
    CanPhy& Phy,
    IEcuPowerStateController& powerStateController)
: AbstractCANTransceiver(busId)
, fdevConfig(devConfig)
, fFlexCANDevice(
      devConfig,
      Phy,
      ::etl::delegate<
          void()>::create<CanFlex2Transceiver, &CanFlex2Transceiver::canFrameSentCallback>(*this),
      powerStateController)
, fIsPhyErrorPresent(false)
, fTxOfflineErrors(0)
, fOverrunCount(0)
, fFramesSentCount(0)
, fTxQueue()
, fFirstFrameNotified(false)
, fRxAlive(false)
, _context(context)
, _cyclicTask(
      ::async::Function::CallType::create<CanFlex2Transceiver, &CanFlex2Transceiver::cyclicTask>(
          *this))
, _cyclicTaskTimeout()
, _canFrameSent(
      ::async::Function::CallType::
          create<CanFlex2Transceiver, &CanFlex2Transceiver::canFrameSentAsyncCallback>(*this))
{
    fpCanTransceivers[fFlexCANDevice.getIndex()] = this;
}

::can::ICanTransceiver::ErrorCode CanFlex2Transceiver::init()
{
    if (State::CLOSED == _state)
    {
        {
            ::async::LockType const lock;
            if (ErrorCode::CAN_ERR_INIT_FAILED == fFlexCANDevice.init())
            {
                return ErrorCode::CAN_ERR_INIT_FAILED;
            }
        }
        (void)fFlexCANDevice.getPhy().setMode(CanPhy::CAN_PHY_MODE_ACTIVE, fdevConfig.BusId);
        _state = State::INITIALIZED;
        return ErrorCode::CAN_ERR_OK;
    }
    return ErrorCode::CAN_ERR_ILLEGAL_STATE;
}

::can::ICanTransceiver::ErrorCode CanFlex2Transceiver::write(::can::CANFrame const& frame)
{
    return write(frame, nullptr);
}

::can::ICanTransceiver::ErrorCode
CanFlex2Transceiver::write(::can::CANFrame const& frame, ::can::ICANFrameSentListener& listener)
{
    return write(frame, &listener);
}

::can::ICanTransceiver::ErrorCode CanFlex2Transceiver::write(
    ::can::CANFrame const& frame, ::can::ICANFrameSentListener* const pListener)
{
    if (State::MUTED == _state)
    {
        logger::Logger::warn(
            logger::CAN,
            "Write Id 0x%x to muted %s",
            frame.getId(),
            ::common::busid::BusIdTraits::getName(_busId));
        return ErrorCode::CAN_ERR_ILLEGAL_STATE;
    }
    if (State::CLOSED == _state)
    {
        fTxOfflineErrors++;
        return ErrorCode::CAN_ERR_TX_OFFLINE;
    }

    ::async::ModifiableLockType mlock;
    if (pListener == nullptr)
    {
        uint8_t const messageBuffer = fFlexCANDevice.getTransmitBuffer(frame, false);
        if (FlexCANDevice::TRANSMIT_BUFFER_UNAVAILABLE == messageBuffer)
        {
            fOverrunCount++;
            mlock.unlock();
            notifyRegisteredSentListener(frame);
            return ErrorCode::CAN_ERR_TX_HW_QUEUE_FULL;
        }

        ErrorCode const status = fFlexCANDevice.transmit(frame, messageBuffer, false);
        if (ErrorCode::CAN_ERR_OK == status)
        {
            fFramesSentCount++;
            mlock.unlock();
            notifyRegisteredSentListener(frame);
            return ErrorCode::CAN_ERR_OK;
        }
        mlock.unlock();
        notifyRegisteredSentListener(frame);
        return ErrorCode::CAN_ERR_TX_FAIL;
    }

    if (fTxQueue.full())
    {
        // no more room for a sender with callback
        fOverrunCount++;
        mlock.unlock();
        notifyRegisteredSentListener(frame);
        return ErrorCode::CAN_ERR_TX_HW_QUEUE_FULL;
    }
    bool const wasEmpty = fTxQueue.empty();
    fTxQueue.emplace_back(*pListener, frame);
    if (!wasEmpty)
    {
        // nothing to do next frame will be sent from tx isr
        return ErrorCode::CAN_ERR_OK;
    }
    ErrorCode status;
    // we are the first sender --> transmit
    uint8_t const messageBuffer = fFlexCANDevice.getTransmitBuffer(frame, true);
    if (messageBuffer != FlexCANDevice::TRANSMIT_BUFFER_UNAVAILABLE)
    {
        status = fFlexCANDevice.transmit(frame, messageBuffer, true);
        if (ErrorCode::CAN_ERR_OK == status)
        {
            // wait until tx interrupt ...
            // ... then canFrameSentCallback() is called
            return ErrorCode::CAN_ERR_OK;
        }
        status = ErrorCode::CAN_ERR_TX_FAIL;
    }
    else
    {
        status = ErrorCode::CAN_ERR_TX_HW_QUEUE_FULL;
    }
    fTxQueue.pop_front();
    mlock.unlock();
    notifyRegisteredSentListener(frame);
    return status;
}

void CanFlex2Transceiver::canFrameSentCallback() { ::async::execute(_context, _canFrameSent); }

void CanFlex2Transceiver::canFrameSentAsyncCallback()
{
    ::async::ModifiableLockType mlock;
    fFramesSentCount++;
    if (!fTxQueue.empty())
    {
        bool sendAgain = false;
        {
            TxJobWithCallback& job                 = fTxQueue.front();
            ::can::CANFrame const& frame           = job._frame;
            ::can::ICANFrameSentListener& listener = job._listener;
            fTxQueue.pop_front();
            if (!fTxQueue.empty())
            {
                // send again only if same precondition as for write() is satisfied!
                if ((State::OPEN == _state) || (State::INITIALIZED == _state))
                {
                    sendAgain = true;
                }
                else
                {
                    fTxQueue.clear();
                }
            }
            mlock.unlock();
            listener.canFrameSent(frame);
            notifyRegisteredSentListener(frame);
        }
        if (sendAgain)
        {
            mlock.lock();
            ::can::CANFrame const& frame = fTxQueue.front()._frame;
            uint8_t const messageBuffer  = fFlexCANDevice.getTransmitBuffer(frame, true);
            if (FlexCANDevice::TRANSMIT_BUFFER_UNAVAILABLE != messageBuffer)
            {
                ErrorCode const status = fFlexCANDevice.transmit(frame, messageBuffer, true);
                if (ErrorCode::CAN_ERR_OK == status)
                {
                    // wait until tx interrupt ...
                    // ... then canFrameSentCallback() is called
                    return;
                }
            }
            // no interrupt will ever retrigger this call => remove all queued frames
            fTxQueue.clear();
            mlock.unlock();
            notifyRegisteredSentListener(frame);
        }
    }
}

uint16_t CanFlex2Transceiver::getHwQueueTimeout() const
{
    // 64 = 8 byte payload
    // 53 = CAN overhead
    auto const timeout = ((53U + 64U) * 1000U / (fFlexCANDevice.getBaudrate()));
    return static_cast<uint16_t>(timeout);
}

uint16_t CanFlex2Transceiver::getFirstFrameId() const
{
    return static_cast<uint16_t>(fFlexCANDevice.getFirstCanId());
}

void CanFlex2Transceiver::resetFirstFrame()
{
    fFlexCANDevice.resetFirstFrame();
    fFirstFrameNotified = false;
}

::can::ICanTransceiver::ErrorCode CanFlex2Transceiver::open(::can::CANFrame const& /* frame */)
{
    // not implemented
    estd_assert(false);
    return ::can::ICanTransceiver::ErrorCode::CAN_ERR_ILLEGAL_STATE;
}

::can::ICanTransceiver::ErrorCode CanFlex2Transceiver::open()
{
    if ((State::INITIALIZED == _state) || (State::CLOSED == _state))
    {
        if (ErrorCode::CAN_ERR_OK == fFlexCANDevice.start())
        {
            (void)fFlexCANDevice.getPhy().setMode(CanPhy::CAN_PHY_MODE_ACTIVE, fdevConfig.BusId);
            _state = State::OPEN;

            ::async::scheduleAtFixedRate(
                _context,
                _cyclicTask,
                _cyclicTaskTimeout,
                ERROR_POLLING_TIMEOUT,
                ::async::TimeUnitType::MILLISECONDS);

            return ErrorCode::CAN_ERR_OK;
        }
        else
        {
            return ErrorCode::CAN_ERR_ILLEGAL_STATE;
        }
    }
    else
    {
        return ErrorCode::CAN_ERR_ILLEGAL_STATE;
    }
}

::can::ICanTransceiver::ErrorCode CanFlex2Transceiver::close()
{
    if ((State::OPEN == _state) || (State::MUTED == _state))
    {
        fFlexCANDevice.stop();
        (void)fFlexCANDevice.getPhy().setMode(CanPhy::CAN_PHY_MODE_STANDBY, fdevConfig.BusId);

        _cyclicTaskTimeout.cancel();

        _state = State::CLOSED;
        {
            ::async::LockType const lock;
            fTxQueue.clear();
        }
        return ErrorCode::CAN_ERR_OK;
    }
    else
    {
        return ErrorCode::CAN_ERR_ILLEGAL_STATE;
    }
}

void CanFlex2Transceiver::shutdown()
{
    (void)close();
    _cyclicTaskTimeout.cancel();
}

::can::ICanTransceiver::ErrorCode CanFlex2Transceiver::mute()
{
    if (State::OPEN == _state)
    {
        fFlexCANDevice.mute();
        {
            async::LockType const lock;
            fTxQueue.clear();
        }
        _state = State::MUTED;
        return ErrorCode::CAN_ERR_OK;
    }
    else
    {
        return ErrorCode::CAN_ERR_ILLEGAL_STATE;
    }
}

::can::ICanTransceiver::ErrorCode CanFlex2Transceiver::unmute()
{
    if (State::MUTED == _state)
    {
        fFlexCANDevice.unmute();
        _state = State::OPEN;
        return ErrorCode::CAN_ERR_OK;
    }
    else
    {
        return ErrorCode::CAN_ERR_ILLEGAL_STATE;
    }
}

void CanFlex2Transceiver::receiveTask()
{
    ::async::ModifiableLockType mlock;
    while (!fFlexCANDevice.isRxQueueEmpty())
    {
        ::can::CANFrame& frame = fFlexCANDevice.getRxFrameQueueFront();
        mlock.unlock();
        notifyListeners(frame);
        mlock.lock();
        fFlexCANDevice.dequeueRxFrame();
    }
}

void CanFlex2Transceiver::cyclicTask()
{
    if (fFlexCANDevice.getBusOffState() == FlexCANDevice::BUS_OFF)
    {
        if (_transceiverState != ::can::ICANTransceiverStateListener::CANTransceiverState::BUS_OFF)
        {
            _transceiverState = ::can::ICANTransceiverStateListener::CANTransceiverState::BUS_OFF;
            notifyStateListenerWithState(_transceiverState);
        }
    }
    else
    {
        if (_transceiverState != ::can::ICANTransceiverStateListener::CANTransceiverState::ACTIVE)
        {
            _transceiverState = ::can::ICANTransceiverStateListener::CANTransceiverState::ACTIVE;
            notifyStateListenerWithState(_transceiverState);
        }
    }

    CanPhy::ErrorCode const phyState = fFlexCANDevice.getPhy().getPhyErrorStatus(fdevConfig.BusId);

    if (phyState != CanPhy::CAN_PHY_ERROR_UNSUPPORTED)
    {
        if (phyState == CanPhy::CAN_PHY_ERROR)
        {
            if (!fIsPhyErrorPresent)
            {
                fIsPhyErrorPresent = true;
                notifyStateListenerWithPhyError();
            }
        }
        else
        {
            fIsPhyErrorPresent = false;
        }
    }
    // Check whether CAN was active since last poll
    if (fFlexCANDevice.getRxAlive() != 0)
    {
        fRxAlive = true;
    }
    else
    {
        fRxAlive = false;
    }
    fFlexCANDevice.clearRxAlive();
}

} // namespace bios
