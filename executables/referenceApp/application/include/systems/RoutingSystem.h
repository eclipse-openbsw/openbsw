/********************************************************************************
 * Copyright (c) 2022 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "routing/ErrorHandler.h"

#include "systems/ICanSystem.h"

#include <async/util/Call.h>
#include <can/filter/IntervalFilter.h> // pre-integration testing, to be removed later
#include <can/transceiver/AbstractCANTransceiver.h>
#include <io/IReader.h>
#include <io/IWriter.h>
#include <io/MemoryQueue.h>
#include <lifecycle/SimpleLifecycleComponent.h>
#include <lwipSocket/udp/LwipDatagramSocket.h>
#include <routing/Integration.h>
#include <routing/PduTransportIntegration.h>
#include <routing/constants.h>

#include <etl/limits.h>
#include <etl/singleton_base.h>
#include <etl/vector.h>
#include <platform/estdint.h>

namespace systems
{
class RoutingSystem
: public ::etl::singleton_base<RoutingSystem>
, public ::lifecycle::SimpleLifecycleComponent
, private ::async::IRunnable
{
public:
    static constexpr auto MAX_NUM_PDU_TRANSPORT_CHANNELS = ::routing::NUM_PDU_TRANSPORT_CHANNELS;
    static constexpr auto NUM_FLEXRAY_CHANNELS           = ::routing::NUM_FLEXRAY_CHANNELS;
    static constexpr auto NUM_CAN_CHANNELS               = ::routing::NUM_CAN_CHANNELS;

    // Ethernet MTU (1500) - IPv4 header size (20) - UDP header size (8) - space for future protocol
    // headers (56)
    static constexpr size_t MAX_FRAME_SIZE                         = 1416U;
    static constexpr size_t MAX_PDU_TRANSPORT_CHANNEL_ELEMENT_SIZE = 72U;
    static constexpr size_t MAX_CAN_CHANNEL_ELEMENT_SIZE           = 72U;
    static constexpr size_t MAX_FLEXRAY_CHANNEL_ELEMENT_SIZE       = 72U;

    RoutingSystem(::async::ContextType context, ::can::ICanSystem& canSystem);

    void init() override;
    void run() override;
    void shutdown() override;

    ::async::ContextType getTransitionContext(Transition::Type /* transition */) override
    {
        return _context;
    }

private:
    static constexpr size_t MAX_CAN_QUEUE_ELEMENT_SIZE = 72U;
    static constexpr size_t CAN_QUEUE_SIZE             = 2048U;
    using CanQueue  = ::io::MemoryQueue<CAN_QUEUE_SIZE, MAX_CAN_QUEUE_ELEMENT_SIZE>;
    using CanReader = ::io::MemoryQueueReader<CanQueue>;
    using CanWriter = ::io::MemoryQueueWriter<CanQueue>;

    class CanRxListener final : public ::can::ICANFrameListener
    {
    public:
        CanRxListener(CanWriter& inputWriter) : _canFilter(), _inputWriter(inputWriter)
        {
            // It will accept any IDs, but Router will filter them.
            // Possible optimization: filtering in listener to avoid spamming the input queue.
            _canFilter.open();
        }

        void frameReceived(::can::CANFrame const& frame) override
        {
            ::etl::span<uint8_t> pdu = _inputWriter.allocate(8U + frame.getPayloadLength());
            if (pdu.empty())
            {
                return;
            }

            pdu.take<::etl::be_uint32_t>() = frame.getId();
            pdu.take<::etl::be_uint32_t>() = frame.getPayloadLength();
            ::etl::copy(
                ::etl::span<uint8_t const>(frame.getPayload(), frame.getPayloadLength()), pdu);

            _inputWriter.commit();
        }

        ::can::IFilter& getFilter() override { return _canFilter; }

    private:
        ::can::IntervalFilter _canFilter;

        CanWriter& _inputWriter;
    };

    class CanOutputWriter final : public ::io::IWriter
    {
    public:
        CanOutputWriter(::can::ICanTransceiver& canTransceiver) : _canTransceiver(canTransceiver) {}

        size_t maxSize() const override { return MAX_CAN_QUEUE_ELEMENT_SIZE; }

        ::etl::span<uint8_t> allocate(size_t size)
        {
            _currentFrame = ::etl::make_span(_data).first(size);
            return _currentFrame;
        }

        void commit()
        {
            if (_currentFrame.empty())
            {
                return;
            }

            uint32_t id     = _currentFrame.take<::etl::be_uint32_t>();
            uint32_t length = _currentFrame.take<::etl::be_uint32_t>();
            auto payload    = _currentFrame;
            ::can::CANFrame frame(id, payload.data(), length);

            // No retry handling
            (void)_canTransceiver.write(frame);

            _currentFrame = {};
        }

        void flush() {}

    private:
        ::can::ICanTransceiver& _canTransceiver;

        ::etl::array<uint8_t, MAX_CAN_QUEUE_ELEMENT_SIZE> _data;
        ::etl::span<uint8_t> _currentFrame;
    };

    using Integration = ::routing::Integration<
        MAX_NUM_PDU_TRANSPORT_CHANNELS,
        NUM_CAN_CHANNELS,
        NUM_FLEXRAY_CHANNELS,
        MAX_PDU_TRANSPORT_CHANNEL_ELEMENT_SIZE,
        MAX_CAN_CHANNEL_ELEMENT_SIZE,
        MAX_FLEXRAY_CHANNEL_ELEMENT_SIZE>;
    using PduTransportIntegration
        = ::routing::PduTransportIntegration<MAX_NUM_PDU_TRANSPORT_CHANNELS, MAX_FRAME_SIZE>;

    void execute() override;

    void
    handleError(::routing::ErrorHandler::StatusCode statusCode, uint8_t channelId, uint32_t data);

    bool _initialized;

    PduTransportIntegration _pduTransportIntegration;

    Integration _integration;

    ::async::TimeoutType _timeout;

    ::async::ContextType _context;

    ::can::ICanSystem& _canSystem;

    ::etl::vector<::udp::LwipDatagramSocket, MAX_NUM_PDU_TRANSPORT_CHANNELS> _lwipInputSockets;
    ::etl::vector<::udp::LwipDatagramSocket, MAX_NUM_PDU_TRANSPORT_CHANNELS> _lwipOutputSockets;
    ::etl::vector<::udp::AbstractDatagramSocket*, MAX_NUM_PDU_TRANSPORT_CHANNELS> _inputSockets;
    ::etl::vector<::udp::AbstractDatagramSocket*, MAX_NUM_PDU_TRANSPORT_CHANNELS> _outputSockets;

    ::etl::vector<CanQueue, NUM_CAN_CHANNELS> _canInputQueues;
    ::etl::vector<CanReader, NUM_CAN_CHANNELS> _canInputReaders;
    ::etl::vector<CanWriter, NUM_CAN_CHANNELS> _canInputWriters;
    ::etl::vector<CanRxListener, NUM_CAN_CHANNELS> _canRxListeners;

    ::etl::vector<CanOutputWriter, NUM_CAN_CHANNELS> _canOutputWriters;

    ::routing::ErrorHandler::Function _errorHandlingFunction;
};

} // namespace systems
