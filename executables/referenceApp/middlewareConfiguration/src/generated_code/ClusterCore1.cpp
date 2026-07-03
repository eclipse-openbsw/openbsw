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
#include "middleware/ClusterCore1.h"

#include <etl/algorithm.h>
#include <etl/type_traits.h>

#include "middleware/ClusterId.h"
#include "middleware/core/LoggerApi.h"
#include "shm/QueueDefinitions.h"

namespace middleware
{
void processCore1Cluster(size_t messages, const bool updateTimeouts)
{
    auto* queue = shm::getQueueToCore1();
    auto core1Receiver =
        etl::remove_pointer<decltype(queue)>::type::Receiver(*queue);
    queue->takeSnapshot();
    if (messages == 0U)
    {
        messages = core1Receiver.size();
    }

    for (size_t i = 0U; (i < messages) && (!core1Receiver.isEmpty()); ++i)
    {
        const core::Message& msg = core1Receiver.peek();
        const auto sourceClusterId = static_cast<core::ClusterId>(msg.getHeader().srcClusterId);
        switch (sourceClusterId)
        {
            case core::ClusterId::Core0: {
                ClusterConnectionCore1ToCore0.processMessage(msg);
                break;
            }
            default: {
                logger::logMessageSendingFailure(
                    logger::LogLevel::Error, logger::Error::DispatchMessage, core::HRESULT::RoutingError, msg);
                break;
            }
        }
        core1Receiver.advance();
    }

    if (updateTimeouts)
    {
        updateCore1ClusterTimeouts();
    }
}

void updateCore1ClusterTimeouts()
{
}

// ***************************** Core1 *****************************
meta::ClusterConnectionCore1ToCore0Meta ClusterConnectionCore1ToCore0Config;
core::ClusterConnectionTypeSelector<meta::ClusterConnectionCore1ToCore0Meta>::type ClusterConnectionCore1ToCore0(
    ClusterConnectionCore1ToCore0Config);

void initializeCore1ClusterConnection()
{
    static_cast<void>(new (&ClusterConnectionCore1ToCore0Config) decltype(ClusterConnectionCore1ToCore0Config)());
    static_cast<void>(new (&ClusterConnectionCore1ToCore0)
        decltype(ClusterConnectionCore1ToCore0)(
            ClusterConnectionCore1ToCore0Config));
}


}  // namespace middleware
