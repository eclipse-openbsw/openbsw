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
#include "foo/FooSkeletonWrapper.h"
#include "middleware/ClusterCore0.h"
#include "shm/QueueDefinitions.h"

void run_main_core0()
{
    middleware::initializeCore0ClusterConnection();

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
        }

        if (counter % 150 == 0)
        {
            auto const stats = middleware::shm::getQueueToCore0()->getStats();
            simulation::Logger::logStats(
                static_cast<uint8_t>(::middleware::core::ClusterId::Core0),
                "Messages [ Written:",
                stats.processedMessages,
                ", Lost: ",
                stats.lostMessages,
                " ]");
        }

        counter++;

        middleware::processCore0Cluster(10);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
