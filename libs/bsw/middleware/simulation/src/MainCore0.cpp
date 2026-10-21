/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/
#include <chrono>
#include <thread>

#include <etl/algorithm.h>
#include <etl/array.h>
#include <etl/bit_stream.h>
#include <etl/crc8_ccitt.h>

#include "Logger.h"
#include "foo/FooSkeletonWrapper.h"
#include "middleware/ClusterCluster0.h"
#include "org/test/signal/samplePduWithDatatypesCommon.h"
#include "shm/PduDefinitions.h"
#include "shm/QueueDefinitions.h"

void write_sample_pdu_with_crc()
{
    constexpr size_t PDU_SIZE = 8U;
    etl::array<uint8_t, PDU_SIZE - 1U> payload{0xDEU, 0xADU, 0xBEU, 0xEFU, 0xDEU, 0xADU, 0xBEU};
    uint8_t const crc = etl::crc8_ccitt(payload.begin(), payload.end());

    etl::array<uint8_t, PDU_SIZE> pduData{crc};
    etl::copy(payload.begin(), payload.end(), pduData.begin() + 1U);

    middleware::shm::getSamplepduwithcrc()->store(pduData);
}

void write_sample_pdu_with_datatypes()
{
    constexpr size_t PDU_SIZE = 12U;
    etl::array<uint8_t, PDU_SIZE> pduData{};
    etl::bit_stream_writer writer(
        pduData.begin(),
        pduData.end(),
        etl::bit_order::msb_first,
        etl::bit_stream_writer::callback_type{},
        etl::endian::big);
    writer.write_unchecked<uint8_t>(0x1DU, 5U);                   // firstSignal
    writer.write_unchecked<uint64_t>(0x0EADBEEFDEADBEEFULL, 60U); // secondSignal
    writer.write_unchecked<uint8_t>(0xDEU, 8U);                   // arraySignal[0]
    writer.write_unchecked<uint8_t>(0xADU, 8U);                   // arraySignal[1]
    writer.write_unchecked<uint8_t>(
        static_cast<uint8_t>(org::test::signal::ExampleEnum::ErrorValue), 7U); // enumSignal
    writer.write_unchecked<uint8_t>(0x15U, 5U);                                // bitsetSignal
    writer.write_unchecked<bool>(true, 1U);                                    // boolSignal
    middleware::shm::getSamplepduwithdatatypes()->store(pduData);
}

void run_main_core0()
{
    middleware::initializeCluster0ClusterConnection();

    FooSkeletonWrapper foo_provider{};
    foo_provider.init();

    size_t counter = 0U;
    for (;;)
    {
        if (counter > 0 && counter % 600 == 0)
        {
            break; // Run simulation for 60 seconds.
        }

        if (counter % 50 == 0)
        {
            // Broadcast a new FooDefault value every 5 seconds.
            foo_provider.sendBroadcast();

            write_sample_pdu_with_crc();
            write_sample_pdu_with_datatypes();
        }

        if (counter % 150 == 0)
        {
            auto const stats = middleware::shm::getQueueToCluster0()->getStats();
            simulation::Logger::logStats(
                static_cast<uint8_t>(::middleware::core::ClusterId::Cluster0),
                "Messages [ Written:",
                stats.processedMessages,
                ", Lost: ",
                stats.lostMessages,
                " ]");
        }

        counter++;

        middleware::processCluster0Cluster(10);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
