/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/
/**
  *
  *  __  __ _     _     _ _
  * |  \/  (_) __| | __| | | _____      ____ _ _ __ ___
  * | |\/| | |/ _` |/ _` | |/ _ \ \ /\ / / _` | '__/ _ \
  * | |  | | | (_| | (_| | |  __/\ V  V / (_| | | |  __/
  * |_|  |_|_|\__,_|\__,_|_|\___| \_/\_/ \__,_|_|  \___|
  *
  * WARNING!
  * This file is generated. Do not edit manually.
  *
  */
#include "middleware/ClusterCore0.h"

#include <etl/algorithm.h>
#include <etl/type_traits.h>

#include "middleware/ClusterId.h"
#include "middleware/core/LoggerApi.h"
#include "shm/QueueDefinitions.h"

namespace middleware
{
void processCore0Cluster(size_t messages, const bool updateTimeouts)
{
    auto* queue = shm::getQueueToCore0();
    auto core0Receiver =
        etl::remove_pointer<decltype(queue)>::type::Receiver(*queue);
    queue->takeSnapshot();
    if (messages == 0U)
    {
        messages = core0Receiver.size();
    }

    for (size_t i = 0U; (i < messages) && (!core0Receiver.isEmpty()); ++i)
    {
        const core::Message& msg = core0Receiver.peek();
        const auto sourceClusterId = static_cast<core::ClusterId>(msg.getHeader().srcClusterId);
        switch (sourceClusterId)
        {
            case core::ClusterId::Core1: {
                ClusterConnectionCore0ToCore1.processMessage(msg);
                break;
            }
            default: {
                logger::logMessageSendingFailure(
                    logger::LogLevel::Error, logger::Error::DispatchMessage, core::HRESULT::RoutingError, msg);
                break;
            }
        }
        core0Receiver.advance();
    }

    if (updateTimeouts)
    {
        updateCore0ClusterTimeouts();
    }
}

void updateCore0ClusterTimeouts()
{
}

// ***************************** Core0 *****************************
meta::ClusterConnectionCore0ToCore1Meta ClusterConnectionCore0ToCore1Config;
core::ClusterConnectionTypeSelector<meta::ClusterConnectionCore0ToCore1Meta>::type ClusterConnectionCore0ToCore1(
    ClusterConnectionCore0ToCore1Config);

void initializeCore0ClusterConnection()
{
    static_cast<void>(new (&ClusterConnectionCore0ToCore1Config) decltype(ClusterConnectionCore0ToCore1Config)());
    static_cast<void>(new (&ClusterConnectionCore0ToCore1)
        decltype(ClusterConnectionCore0ToCore1)(
            ClusterConnectionCore0ToCore1Config));
}


}  // namespace middleware
