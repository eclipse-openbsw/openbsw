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

#include "Logger.h"
#include "foo/FooProxyWrapper.h"
#include "middleware/ClusterCluster1.h"
#include "shm/QueueDefinitions.h"

void run_main_core1()
{
    middleware::initializeCluster1ClusterConnection();

    FooProxyWrapper<middleware::core::ClusterId::Cluster1> foo_consumer{};
    foo_consumer.init();

    size_t counter = 0U;
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
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    foo_consumer.deInit();
}
