/********************************************************************************
 * Copyright (c) 2024 Roland Reichwein
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "can/transceiver/can/CanTransceiver.h"

#include <async/Types.h>
#include <can/CanLogger.h>
#include <can/framemgmt/IFilteredCANFrameSentListener.h>
#include <common/busid/BusId.h>
#include <etl/delegate.h>
#include <etl/error_handler.h>

#include <platform/config.h>
#include <cstdint>

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
//, fdevConfig(devConfig)
, fFlexCANDevice(
      devConfig,
      Phy,
      ::etl::delegate<
          void()>::create<CanFlex2Transceiver, &CanFlex2Transceiver::canFrameSentCallback>(*this),
      powerStateController)
//, fIsPhyErrorPresent(false)
//, fTxOfflineErrors(0)
//, fOverrunCount(0)
//, fFramesSentCount(0)
, fTxQueue()
//, fFirstFrameNotified(false)
//, fRxAlive(false)
//, _context(context)
, _cyclicTask(
      ::async::Function::CallType::create<CanFlex2Transceiver, &CanFlex2Transceiver::cyclicTask>(
          *this))
, _cyclicTaskTimeout()
, _canFrameSent(
      ::async::Function::CallType::
          create<CanFlex2Transceiver, &CanFlex2Transceiver::canFrameSentAsyncCallback>(*this))
{
    (void)context; // _context is not wired up yet
    // fpCanTransceivers[fFlexCANDevice.getIndex()] = this;
}

::can::ICanTransceiver::ErrorCode CanFlex2Transceiver::init()
{
    return ErrorCode::CAN_ERR_ILLEGAL_STATE;
}

::can::ICanTransceiver::ErrorCode CanFlex2Transceiver::write(::can::CANFrame const& frame)
{
    (void)frame; // not implemented yet
    // return write(frame, nullptr);
    return {};
}

::can::ICanTransceiver::ErrorCode
CanFlex2Transceiver::write(::can::CANFrame const& frame, ::can::ICANFrameSentListener& listener)
{
    (void)frame;    // not implemented yet
    (void)listener; // not implemented yet
    // return write(frame, &listener);
    return {};
}

::can::ICanTransceiver::ErrorCode CanFlex2Transceiver::write(
    ::can::CANFrame const& frame, ::can::ICANFrameSentListener* const pListener)
{
    (void)frame;     // not implemented yet
    (void)pListener; // not implemented yet
    return {};
}

void CanFlex2Transceiver::canFrameSentCallback()
{
    //::async::execute(_context, _canFrameSent);
}

void CanFlex2Transceiver::canFrameSentAsyncCallback() {}

uint16_t CanFlex2Transceiver::getHwQueueTimeout() const { return {}; }

uint16_t CanFlex2Transceiver::getFirstFrameId() const
{
    //    return static_cast<uint16_t>(fFlexCANDevice.getFirstCanId());
    return {};
}

void CanFlex2Transceiver::resetFirstFrame() {}

::can::ICanTransceiver::ErrorCode CanFlex2Transceiver::open(::can::CANFrame const& /* frame */)
{
    return {};
}

::can::ICanTransceiver::ErrorCode CanFlex2Transceiver::open() { return {}; }

::can::ICanTransceiver::ErrorCode CanFlex2Transceiver::close() { return {}; }

void CanFlex2Transceiver::shutdown() {}

::can::ICanTransceiver::ErrorCode CanFlex2Transceiver::mute() { return {}; }

::can::ICanTransceiver::ErrorCode CanFlex2Transceiver::unmute() { return {}; }

void CanFlex2Transceiver::receiveTask() {}

void CanFlex2Transceiver::cyclicTask() {}

} // namespace bios
