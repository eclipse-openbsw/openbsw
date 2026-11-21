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

#include <etl/array.h>
#include <etl/crc8_ccitt.h>

#include "Logger.h"
#include "foo/FooSkeletonWrapper.h"
#include "middleware/Cluster0Cluster.h"
#include "middleware/shm/PduDefinitions.h"
#include "middleware/shm/QueueDefinitions.h"
#include "org/test/signal/samplePduWithCrcWriter.h"
#include "org/test/signal/samplePduWithDatatypesWriter.h"

void write_sample_pdu_with_crc(org::test::signal::samplePduWithCrcWriter& writer)
{
    // The CRC covers the seven bytes after it, which the signals below spell out: 0xDE 0xAD 0xBE
    // 0xEF 0xDE 0xAD 0xBE.
    etl::array<uint8_t, 7U> payload{0xDEU, 0xADU, 0xBEU, 0xEFU, 0xDEU, 0xADU, 0xBEU};

    writer.writeCrc(etl::crc8_ccitt(payload.begin(), payload.end()));
    writer.writeSampleSignal(0xDU);
    writer.writeAnotherSampleSignal(0xEU);
    writer.writeSomeOtherSignal(0xADBEU);
    writer.writeFinalSampleSignal(0xEFDEADBEU);
    writer.commit();
    writer.triggerPduEvent();
}

void write_sample_pdu_with_datatypes(org::test::signal::samplePduWithDatatypesWriter& writer)
{
    using namespace org::test::signal;

    writer.writeSecondSignal(0x0EADBEEFDEADBEEFULL);
    writer.writeArraySignal({0xDEU, 0xADU});
    writer.writeEnumSignal(ExampleEnum::ErrorValue);
    writer.writeBitsetSignal(ExampleBitset{0x15U});
    writer.writeBoolSignal(true);
    writer.writeFirstSignal(0x1DU);
    writer.commit();
}

void run_main_core0()
{
    middleware::initializeCluster0ClusterConnection();

    FooSkeletonWrapper foo_provider{};
    foo_provider.init();

    org::test::signal::samplePduWithCrcWriter crcWriter{
        org::test::signal::samplePduWithCrc{0U, 0U, 0U, 0U, 0U}};
    org::test::signal::samplePduWithDatatypesWriter datatypesWriter{
        org::test::signal::samplePduWithDatatypes{
            0U,
            0U,
            {0U, 0U},
            org::test::signal::ExampleEnum::Value0,
            org::test::signal::ExampleBitset{0U},
            false}};
    crcWriter.commit();
    datatypesWriter.commit();

    constexpr size_t TICKS_PER_SECOND{1000U};
    constexpr size_t SIMULATION_TICKS{60U * TICKS_PER_SECOND};
    constexpr size_t BROADCAST_TICKS{5U * TICKS_PER_SECOND};
    constexpr size_t STATS_TICKS{15U * TICKS_PER_SECOND};
    auto nextTick = std::chrono::steady_clock::now();
    for (size_t counter = 0U; counter < SIMULATION_TICKS; ++counter)
    {
        if (counter % BROADCAST_TICKS == 0U)
        {
            // Broadcast a new FooDefault value every 5 seconds.
            foo_provider.sendBroadcast();

            write_sample_pdu_with_crc(crcWriter);
            write_sample_pdu_with_datatypes(datatypesWriter);
        }

        if (counter % STATS_TICKS == 0U)
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

        middleware::processCluster0Cluster(10);
        nextTick += std::chrono::milliseconds(1);
        std::this_thread::sleep_until(nextTick);
    }
}
