/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include <thread>

#include "Logger.h"
#include "foo/FooProxyWrapper.h"
#include "middleware/ClusterCluster1.h"
#include "middleware/signal/qualifier.h"
#include "org/test/signal/samplePduWithCrcReader.h"
#include "org/test/signal/samplePduWithDatatypesReader.h"
#include "shm/QueueDefinitions.h"

void readsamplePduWithCrc()
{
    middleware::signal::integration::Qualifier qualifier{};
    org::test::signal::samplePduWithCrcReader samplePduWithCrcReader{&qualifier};
    auto const samplePdu = samplePduWithCrcReader.read();
    if (qualifier.getQualification()
        == middleware::signal::integration::Qualifier::Qualification::Ok)
    {
        simulation::Logger::log(
            static_cast<uint8_t>(::middleware::core::ClusterId::Cluster1),
            "Sample PDU [CRC: ",
            "0x",
            std::hex,
            std::uppercase,
            std::setfill('0'),
            std::setw(2),
            static_cast<unsigned int>(samplePdu.crc),
            ", sampleSignal: 0x",
            std::setw(2),
            static_cast<unsigned int>(samplePdu.sampleSignal),
            ", anotherSampleSignal: 0x",
            std::setw(2),
            static_cast<unsigned int>(samplePdu.AnotherSampleSignal),
            ", someOtherSignal: 0x",
            std::setw(4),
            samplePdu.SomeOtherSignal,
            ", finalSampleSignal: 0x",
            std::setw(static_cast<int>(sizeof(samplePdu.finalSampleSignal) * 2U)),
            samplePdu.finalSampleSignal,
            "]");
    }
    else
    {
        simulation::Logger::log(
            static_cast<uint8_t>(::middleware::core::ClusterId::Cluster1),
            "Sample PDU CRC invalid");
    }
}

void readsamplePduWithDatatypes()
{
    org::test::signal::samplePduWithDatatypesReader samplePduWithDatatypesReader{nullptr};
    auto const samplePdu = samplePduWithDatatypesReader.read();

    // No need to check qualifier here, since this PDU does not have CRC attached.
    // Simulation purpose only to check data types

    simulation::Logger::log(
        static_cast<uint8_t>(::middleware::core::ClusterId::Cluster1),
        "PDU With Datatypes read: ",
        "0x",
        std::hex,
        std::uppercase,
        std::setfill('0'),
        std::setw(2),
        static_cast<unsigned int>(samplePdu.firstSignal),
        ", secondSignal: 0x",
        std::setw(16),
        samplePdu.secondSignal,
        ", arraySignal: 0x",
        std::setw(2),
        static_cast<unsigned int>(samplePdu.arraySignal[0]),
        std::setw(2),
        static_cast<unsigned int>(samplePdu.arraySignal[1]),
        ", enumSignal: ",
        std::dec,
        static_cast<unsigned int>(samplePdu.enumSignal),
        ", bitsetSignal [enabled: ",
        samplePdu.bitsetSignal.testEnabled(),
        ", fault: ",
        samplePdu.bitsetSignal.testFault(),
        ", active: ",
        samplePdu.bitsetSignal.testActive(),
        ", locked: ",
        samplePdu.bitsetSignal.testLocked(),
        ", overheated: ",
        samplePdu.bitsetSignal.testOverheated(),
        ", boolSignal: ",
        samplePdu.boolSignal,
        "]");
}

void run_main_core1()
{
    middleware::initializeCluster1ClusterConnection();

    FooProxyWrapper<middleware::core::ClusterId::Cluster1> foo_consumer{};
    foo_consumer.init();

    size_t counter               = 0U;
    uint32_t fooMethodStartValue = 0U;
    for (;;)
    {
        if (counter > 0 && counter % 600 == 0)
        {
            break; // Run simulation for 60 seconds.
        }

        if (counter % 60 == 0)
        {
            // Request current FooDefault value every 6 seconds.
            foo_consumer.requestGet();
            auto const fooMethodRequest = foo_consumer.requestFooMethod(fooMethodStartValue);
            if (fooMethodRequest == middleware::core::HRESULT::Ok)
            {
                ++fooMethodStartValue;
            }
            readsamplePduWithCrc();
            readsamplePduWithDatatypes();
        }

        if (counter % 150 == 0)
        {
            auto const stats = middleware::shm::getQueueToCluster1()->getStats();
            simulation::Logger::logStats(
                static_cast<uint8_t>(::middleware::core::ClusterId::Cluster1),
                "Messages [ Written:",
                stats.processedMessages,
                ", Lost: ",
                stats.lostMessages,
                " ]");
        }

        counter++;

        middleware::processCluster1Cluster(10);
        auto const fooMethodResult = foo_consumer.pollFooMethod();
        using FooMethodState       = middleware::core::Future::State;
        if (fooMethodResult.state == FooMethodState::Ready)
        {
            auto const& values = fooMethodResult.value;
            simulation::Logger::log(
                static_cast<uint8_t>(::middleware::core::ClusterId::Cluster1),
                "FooMethod result, values=[",
                values[0],
                ", ",
                values[1],
                ", ",
                values[2],
                ", ",
                values[3],
                "]");
        }
        else if (
            fooMethodResult.state != FooMethodState::Invalid
            && fooMethodResult.state != FooMethodState::Pending)
        {
            simulation::Logger::log(
                static_cast<uint8_t>(::middleware::core::ClusterId::Cluster1),
                "FooMethod result failed, error=",
                static_cast<unsigned>(fooMethodResult.state));
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    foo_consumer.deInit();
}
