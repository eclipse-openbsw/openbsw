// Copyright 2024 Accenture.

#include "can/FlexCANDevice.h"

#include "can/canframes/CANFrame.h"
#include "can/transceiver/ICanTransceiver.h"
#include "interrupts/SuspendResumeAllInterruptsScopedLock.h"

#include <bsp/timer/SystemTimer.h>
#include <bsp/timer/isEqualAfterTimeout.h>
#include <etl/byte_stream.h>
#include <etl/span.h>
#include <etl/unaligned_type.h>

#include <cassert>

using namespace can;

namespace bios
{

FlexCANDevice::FlexCANDevice(
    Config const& config,
    CanPhy& canPhy,
    ::etl::delegate<void()> frameSentCallback,
    IEcuPowerStateController& powerManager)
: fConfig(config)
, fPhy(canPhy)
, fpDevice(reinterpret_cast<FLEXCAN_Type*>(fConfig.baseAddress))
, fLastMessageBuffer(FIRST_TRANSMIT_BUFFER)
, fTxInterruptMask0(1UL << (CALLBACK_TRANSMIT_BUFFER - (e_TRANSMIT_BUFFER_START)))
, fRxInterruptMask(0)
, fRxQueue()
, fFirstRxId(0)
, fFramesReceived(0)
, fFramesReceivedTotal(0)
, fFrameSentCallback(frameSentCallback)
, fPowerManager(powerManager)
, fBusOff(false)
{
    switch (fConfig.baseAddress)
    {
        case 0x40024000UL: fIndex = 0; break;
        case 0x40025000UL: fIndex = 1; break;
        case 0x4002B000UL: fIndex = 2; break;

        default: assert(1);
    }

    // Buffers 0..16 are used for reception
    assert(fConfig.numRxBufsExt + fConfig.numRxBufsStd <= FIRST_TRANSMIT_BUFFER);

    // Buffers 16..31 are used for transmission (application),
    // buffer CALLBACK_TRANSMIT_BUFFER for transport transmission
    assert(fConfig.numTxBufsApp <= 31);
}

FlexCANDevice::FlexCANDevice(
    Config const& config, CanPhy& canPhy, IEcuPowerStateController& powerManager)
: fConfig(config)
, fPhy(canPhy)
, fpDevice(reinterpret_cast<FLEXCAN_Type*>(fConfig.baseAddress))
, fLastMessageBuffer(FIRST_TRANSMIT_BUFFER)
, fTxInterruptMask0(1UL << (CALLBACK_TRANSMIT_BUFFER - (e_TRANSMIT_BUFFER_START)))
, fRxInterruptMask(0)
, fRxQueue()
, fFirstRxId()
, fFramesReceived(0)
, fFramesReceivedTotal(0)
, fFrameSentCallback()
, fPowerManager(powerManager)
, fBusOff(false)
{
    switch (fConfig.baseAddress)
    {
        case 0x40024000UL: fIndex = 0; break;
        case 0x40025000UL: fIndex = 1; break;
        case 0x4002B000UL: fIndex = 2; break;

        default: assert(1);
    }
    // Buffers 0..16 are used for reception
    assert(fConfig.numRxBufsExt + fConfig.numRxBufsStd <= FIRST_TRANSMIT_BUFFER);

    // Buffers 16..31 are used for transmission (application),
    // buffer CALLBACK_TRANSMIT_BUFFER for transport transmission
    assert(fConfig.numTxBufsApp <= 31);
}

ICanTransceiver::ErrorCode FlexCANDevice::init()
{
    Io::setDefaultConfiguration(fConfig.txPort);
    Io::setDefaultConfiguration(fConfig.rxPort);
    fPhy.init(fConfig.BusId);

    // Enable module
    fpDevice->MCR = fpDevice->MCR & ~FLEXCAN_MCR_MDIS_MASK;

    // Soft reset
    fpDevice->MCR = fpDevice->MCR | FLEXCAN_MCR_SOFTRST_MASK;
    if (::bsp::isEqualAfterTimeout<vuint32_t>(
            &fpDevice->MCR,
            FLEXCAN_MCR_SOFTRST_MASK,
            FLEXCAN_MCR_SOFTRST_MASK,
            INIT_DELAY_TIMEOUT_US))
    {
        return ICanTransceiver::ErrorCode::CAN_ERR_INIT_FAILED;
    }

    // Enter freeze mode
    fpDevice->MCR = fpDevice->MCR | (FLEXCAN_MCR_FRZ_MASK | FLEXCAN_MCR_HALT_MASK);
    if (::bsp::isEqualAfterTimeout<vuint32_t>(
            &fpDevice->MCR, FLEXCAN_MCR_FRZACK_MASK, 0UL, INIT_DELAY_TIMEOUT_US))
    {
        return ICanTransceiver::ErrorCode::CAN_ERR_INIT_FAILED;
    }

    // Setup MCR:
    // Disable self reception
    // IRQM have to be switched on
    fpDevice->MCR = fpDevice->MCR
                    | (FLEXCAN_MCR_MAXMB(e_TRANSMIT_BUFFER_MAX) | FLEXCAN_MCR_SRXDIS_MASK
                       | FLEXCAN_MCR_IRMQ_MASK);

    // Setup CTRL
    fpDevice->CTRL1 = 0;
    fpDevice->CTRL1 = fpDevice->CTRL1 | fConfig.clockSetupRegister;
    fpDevice->CTRL2 = fpDevice->CTRL2 | FLEXCAN_CTRL2_MRP_MASK;

    // Setup message buffers
    fRxInterruptMask = 0;
    uint32_t mask    = 1;
    for (uint8_t i = 0; i < fConfig.numRxBufsStd; ++i)
    {
        setupMessageBuffer(i, CANRxBuffer::CODE_EMPTY, false);
        fRxInterruptMask |= mask;
        mask <<= 1;
    }
    mask = (1 << fConfig.numRxBufsStd);
    for (uint8_t i = fConfig.numRxBufsStd; i < fConfig.numRxBufsStd + fConfig.numRxBufsExt; ++i)
    {
        setupMessageBuffer(i, CANRxBuffer::CODE_EMPTY, true);
        fRxInterruptMask |= mask;
        mask <<= 1;
    }
    for (uint8_t i = FIRST_TRANSMIT_BUFFER; i < FIRST_TRANSMIT_BUFFER + fConfig.numTxBufsApp; ++i)
    {
        setupMessageBuffer(i, CANTxBuffer::CODE_INACTIVE, false);
    }
    setupMessageBuffer(CALLBACK_TRANSMIT_BUFFER, CANTxBuffer::CODE_INACTIVE, false);

    // Clear and disable interrupts
    fpDevice->RXMGMASK = 0;
    fpDevice->RX14MASK = 0;
    fpDevice->RX15MASK = 0;

    fpDevice->IMASK1 = 0;

    fpDevice->IFLAG1 = 0xffffffff;

    fpDevice->ESR1 = 0xffffffff;

    fFirstRxId           = 0;
    fFramesReceived      = 0;
    fFramesReceivedTotal = 0;
    return ICanTransceiver::ErrorCode::CAN_ERR_OK;
}

ICanTransceiver::ErrorCode FlexCANDevice::start()
{
    // Clear interrupt flags
    fpDevice->IFLAG1 = 0xffffffff;

    // Enable receive interrupts
    fpDevice->IMASK1 = fRxInterruptMask;

    // Leave freeze mode
    fpDevice->MCR = fpDevice->MCR & (~FLEXCAN_MCR_HALT_MASK & ~FLEXCAN_MCR_FRZ_MASK);
    if (::bsp::isEqualAfterTimeout<vuint32_t>(
            &fpDevice->MCR,
            (FLEXCAN_MCR_NOTRDY_MASK),
            (FLEXCAN_MCR_NOTRDY_MASK),
            INIT_DELAY_TIMEOUT_US))
    {
        return ICanTransceiver::ErrorCode::CAN_ERR_ILLEGAL_STATE;
    }

    fBusOff            = false;
    fLastMessageBuffer = FIRST_TRANSMIT_BUFFER;
    fFirstRxId         = 0;
    fFramesReceived    = 0;
    fPowerManager.clearWakeupSourceMonitoring(fConfig.wakeUp);
    return ICanTransceiver::ErrorCode::CAN_ERR_OK;
}

void FlexCANDevice::stop()
{
    interrupts::SuspendResumeAllInterruptsScopedLock lock;
    fpDevice->IMASK1 = 0;

    fpDevice->IFLAG1 = 0xffffffff;

    fPowerManager.setWakeupSourceMonitoring(fConfig.wakeUp, true, true);
}

void FlexCANDevice::mute()
{
    for (uint8_t i = FIRST_TRANSMIT_BUFFER; i < FIRST_TRANSMIT_BUFFER + fConfig.numTxBufsApp; ++i)
    {
        messageBuffer(i).FLAGS.B.CODE = CANTxBuffer::CODE_INACTIVE;
    }
    messageBuffer(CALLBACK_TRANSMIT_BUFFER).FLAGS.B.CODE = CANTxBuffer::CODE_INACTIVE;
}

uint8_t FlexCANDevice::receiveISR(uint8_t const* filterMap)
{
    uint32_t currentInterruptFlag = 1;
    uint8_t bufIdx                = 0;
    uint32_t interruptsToProcessMask;
    uint32_t interruptProcessEnableMask = 0xffffffff;
    uint8_t framesReceived              = 0;
    uint32_t volatile dummyRegisterValue; // used for lock/unlock

    fBusOff = false; // bus is not in off state when frames are received

    interruptsToProcessMask = fpDevice->IFLAG1; // get current pending interrupts
    interruptsToProcessMask &= fRxInterruptMask;

    while (interruptsToProcessMask > 0)
    {
        fFramesReceivedTotal++;
        if (interruptsToProcessMask & currentInterruptFlag)
        {
            do
            {
                dummyRegisterValue = messageBuffer(bufIdx).FLAGS.R; // lock buffer
            } while (dummyRegisterValue & CANRxBuffer::FLAG_BUSY);
            fpDevice->IFLAG1 = currentInterruptFlag; // clear interrupt

            uint32_t id = messageBuffer(bufIdx).FLAGS.B.IDE == 0
                              ? messageBuffer(bufIdx).ID.B.ID_STD
                              : (messageBuffer(bufIdx).ID.R & can::CANFrame::MAX_FRAME_ID_EXTENDED);
            if (fFirstRxId == 0)
            {
                fFirstRxId = id;
            }
            framesReceived += enqueueRxFrame(
                id,
                messageBuffer(bufIdx).FLAGS.B.DLC,
                const_cast<vuint32_t*>(messageBuffer(bufIdx).DATA.W),
                messageBuffer(bufIdx).FLAGS.B.IDE == 1,
                filterMap);
        }
        currentInterruptFlag <<= 1;
        interruptProcessEnableMask <<= 1;
        interruptsToProcessMask = fpDevice->IFLAG1 & interruptProcessEnableMask;
        interruptsToProcessMask &= fRxInterruptMask;
        ++bufIdx;
    }
    dummyRegisterValue = fpDevice->TIMER; // unlock buffer
    fFramesReceived += framesReceived;
    return framesReceived;
}

uint8_t FlexCANDevice::enqueueRxFrame(
    uint32_t id, uint8_t length, vuint32_t payload[], bool extended, uint8_t const* filterMap)
{
    if (!fRxQueue.full())
    {
        bool acceptRxFrame = true;
        if ((nullptr != filterMap) && !extended)
        {
            // apply filter only for std. IDs
            uint32_t index = id / 8U;
            uint8_t mask   = 1U << (id % 8U);
            if ((filterMap[index] & mask) == 0)
            {
                acceptRxFrame = false;
            }
        }
        if (acceptRxFrame)
        {
            can::CANFrame& frame{fRxQueue.emplace(CanId::id(id, extended))};
            frame.setTimestamp(getSystemTimeUs32Bit());
            frame.setPayloadLength(length);
            etl::be_uint32_ext_t{frame.getPayload()}                            = payload[0];
            etl::be_uint32_ext_t{frame.getPayload() + sizeof(etl::be_uint32_t)} = payload[1];
            return 1;
        }
    }
    return 0;
}

uint8_t FlexCANDevice::getTransmitBuffer(CANFrame const& frame, bool callbackRequested)
{
    uint8_t free         = TRANSMIT_BUFFER_UNAVAILABLE;
    bool busyBufferFound = false;

    if (callbackRequested)
    {
        if (CANTxBuffer::CODE_INACTIVE == messageBuffer(CALLBACK_TRANSMIT_BUFFER).FLAGS.B.CODE)
        {
            return CALLBACK_TRANSMIT_BUFFER;
        }
        else
        {
            return TRANSMIT_BUFFER_UNAVAILABLE;
        }
    }

    uint32_t frameId = CanId::rawId(frame.getId());
    for (uint8_t bufIdx = fLastMessageBuffer; bufIdx >= FIRST_TRANSMIT_BUFFER; --bufIdx)
    {
        if (CANTxBuffer::CODE_INACTIVE == messageBuffer(bufIdx).FLAGS.B.CODE)
        {
            free = bufIdx;
            if (!busyBufferFound)
            {
                fLastMessageBuffer = free;
            }
        }
        else
        {
            busyBufferFound = true;

            bool extendedBufferId = (messageBuffer(bufIdx).FLAGS.B.IDE == 1);
            uint32_t bufId
                = extendedBufferId ? messageBuffer(bufIdx).ID.R : messageBuffer(bufIdx).ID.B.ID_STD;
            if ((bufId == frameId) && (extendedBufferId == CanId::isExtended(frame.getId())))
            { // buffer with same id
                if (TRANSMIT_BUFFER_UNAVAILABLE == free)
                {
                    if (fLastMessageBuffer < static_cast<uint32_t>(
                            (FIRST_TRANSMIT_BUFFER + fConfig.numTxBufsApp) - 1U))
                    {
                        free = ++fLastMessageBuffer;
                    }
                }
                return free;
            }
        }
    }

    if (TRANSMIT_BUFFER_UNAVAILABLE == free)
    {
        if (fLastMessageBuffer
            < static_cast<uint32_t>((FIRST_TRANSMIT_BUFFER + fConfig.numTxBufsApp) - 1U))
        {
            free = ++fLastMessageBuffer;
        }
    }
    return free;
}

ICanTransceiver::ErrorCode
FlexCANDevice::transmit(CANFrame const& frame, uint8_t bufIdx, bool txInterruptNeeded)
{
    interrupts::SuspendResumeAllInterruptsScopedLock lock;

    if (((bufIdx >= FIRST_TRANSMIT_BUFFER)
         && (bufIdx < FIRST_TRANSMIT_BUFFER + fConfig.numTxBufsApp))
        || (CALLBACK_TRANSMIT_BUFFER == bufIdx))
    {
        if (CANTxBuffer::CODE_INACTIVE == messageBuffer(bufIdx).FLAGS.B.CODE)
        {
            if (txInterruptNeeded)
            {
                fTxInterruptMask0 = 0;
                fTxInterruptMask0
                    |= (1 << (bufIdx - e_TRANSMIT_BUFFER_START + FIRST_TRANSMIT_BUFFER));
            }
            if (CanId::isExtended(frame.getId()))
            {
                messageBuffer(bufIdx).ID.R        = CanId::rawId(frame.getId());
                messageBuffer(bufIdx).FLAGS.B.IDE = 1;
                messageBuffer(bufIdx).FLAGS.B.SRR = 1;
            }
            else
            {
                messageBuffer(bufIdx).ID.B.ID_STD = CanId::rawId(frame.getId());
                messageBuffer(bufIdx).FLAGS.B.IDE = 0;
            }
            if (frame.getPayloadLength() >= 8)
            {
                ::etl::byte_stream_reader byte_stream{frame.getPayload(), 8, etl::endian::big};
                messageBuffer(bufIdx).DATA.W[0]   = byte_stream.read_unchecked<uint32_t>();
                messageBuffer(bufIdx).DATA.W[1]   = byte_stream.read_unchecked<uint32_t>();
                messageBuffer(bufIdx).FLAGS.B.DLC = frame.getPayloadLength();
                if (txInterruptNeeded)
                {
                    enableTransmitInterrupt();
                }
                messageBuffer(bufIdx).FLAGS.B.CODE = CANTxBuffer::CODE_TRANSMIT;
                return ICanTransceiver::ErrorCode::CAN_ERR_OK;
            }
            else
            {
                return ICanTransceiver::ErrorCode::CAN_ERR_TX_FAIL;
            }
        }
        else
        {
            return ICanTransceiver::ErrorCode::CAN_ERR_TX_FAIL;
        }
    }
    else
    {
        return ICanTransceiver::ErrorCode::CAN_ERR_TX_FAIL;
    }
}

unsigned char FlexCANDevice::dequeueRxFrameStream(unsigned char* data)
{
    uint32_t id;
    unsigned char* ptr;
    unsigned char i;
    if (isRxQueueEmpty())
    {
        return 0;
    }
    else
    {
        can::CANFrame& frame = fRxQueue.front();
        data[0]              = frame.getPayloadLength();
        id                   = frame.getId();
        data[1]              = static_cast<unsigned char>(id >> 8);
        data[2]              = static_cast<unsigned char>(id);
        ptr                  = frame.getPayload();
        for (i = 0; i < data[0]; ++i)
        {
            data[3 + i] = ptr[i];
        }
        fRxQueue.pop();
        return 1;
    }
}

ICanTransceiver::ErrorCode
FlexCANDevice::transmitStream(uint8_t* /* Txframe */, bool /* txInterruptNeeded */)
{
    return ICanTransceiver::ErrorCode::CAN_ERR_TX_FAIL;
}

bool FlexCANDevice::wokenUp()
{
    bool ret = true;
    if (fpDevice->IMASK1 == 0)
    {
        // CAN Device in Stop
        if ((fPowerManager.getWakeupSource() & fConfig.wakeUp) == 0)
        {
            ret = false;
        }
    }
    else
    {
        {
            interrupts::SuspendResumeAllInterruptsScopedLock lock;
            if (fFramesReceivedTotal == 0)
            {
                ret = false;
            }
            fFramesReceivedTotal = 0;
        }
    }
    return ret;
}

void FlexCANDevice::transmitISR()
{
    disableTransmitInterrupt();
    fFrameSentCallback();
}

} // namespace bios
