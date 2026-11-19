/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/
#pragma once

#include <etl/algorithm.h>
#include <etl/array.h>
#include <etl/span.h>
#include <middleware/sig/core/IPduTxSink.h>
#include <util/logger/Logger.h>

#include "middleware/shm/PduDefinitions.h"
#include "middleware/shm/QueueDefinitions.h"
#include "org/test/pdu/demoRxPduReader.h"
#include "org/test/pdu/demoTxPduWriter.h"
#include <app/DemoLogger.h>

/**
 * Writes a PDU, lets the PDU broker transmit it to a loopback bus and receives it again as a
 * different PDU.
 */
class PduDemoWrapper
{
public:
    /** \brief Writes the next counter value; the next process() call transmits it. */
    void sendPdu()
    {
        ++_counter;
        _writer.writeCounter(_counter);
        _writer.commit();
        ::util::logger::Logger::info(
            ::util::logger::DEMO, "[Middleware] PDU written, counter=%u", _counter);
    }

    /** \brief Transmits the committed PDUs and receives what the loopback bus returns. */
    void process()
    {
        _broker.processTransmissions();
        if (_bus.loopBack(_broker))
        {
            ::util::logger::Logger::info(
                ::util::logger::DEMO,
                "[Middleware] PDU received, counter=%u",
                _reader.read().counter);
        }
    }

private:
    // Keeps the transmitted PDU, so that loopBack() can receive it as the Rx PDU.
    class LoopbackBus final : public ::middleware::sig::core::IPduTxSink
    {
    public:
        void submit(uint32_t const id, ::etl::span<uint8_t const> const payload) override
        {
            if (_pending || payload.size() != _frame.size())
            {
                ::util::logger::Logger::warn(
                    ::util::logger::DEMO, "[Middleware] PDU dropped, id=%u", id);
                return;
            }
            ::etl::copy(payload.begin(), payload.end(), _frame.begin());
            _pending = true;
            ::util::logger::Logger::info(
                ::util::logger::DEMO, "[Middleware] PDU transmitted, id=%u", id);
        }

        /** \return true if a PDU was received. */
        bool loopBack(::middleware::sig::core::PduBrokerConnectionBase& broker)
        {
            if (!_pending)
            {
                return false;
            }
            _pending = false;
            return broker.onReceive(
                ::middleware::shm::PDU_ID_DEMORXPDU, ::etl::span<uint8_t const>(_frame));
        }

    private:
        ::etl::array<uint8_t, 4U> _frame{};
        bool _pending{false};
    };

    LoopbackBus _bus;
    ::middleware::shm::PduBrokerConnection _broker{
        _bus, ::middleware::shm::getPduAccessTable(), *::middleware::shm::getOutboundPduQueue()};
    org::test::pdu::demoTxPduWriter _writer{org::test::pdu::demoTxPdu{0U}};
    org::test::pdu::demoRxPduReader _reader{nullptr};
    uint32_t _counter{0U};
};
