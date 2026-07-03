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
#pragma once

#include <etl/array.h>
#include <etl/iterator.h>
#include <etl/type_traits.h>
#include <etl/utility.h>
#include <etl/vector.h>

#include "middleware/ClusterId.h"
#include "middleware/core/ClusterConnection.h"
#include "middleware/core/DatabaseManipulator.h"
#include "middleware/core/IClusterConnectionConfigurationBase.h"
#include "middleware/core/Message.h"
#include "middleware/core/TransceiverContainer.h"
#include "middleware/core/types.h"
#include "shm/QueueDefinitions.h"
#include "org/test/foo/FooCommon.h"

namespace middleware::meta
{

class ClusterConnectionCore1ToCore0Meta : public core::IClusterConnectionConfigurationProxyOnly
{
public:
    using Base = core::IClusterConnectionConfigurationProxyOnly;

    ClusterConnectionCore1ToCore0Meta()
    : proxyTransceivers_{{
                {&orgtestfooFooProxyTransceivers_, org::test::foo::Foo::internal::SERVICE_ID, 0U}
      }}
    {
    }

    ClusterConnectionCore1ToCore0Meta(const ClusterConnectionCore1ToCore0Meta&) = delete;
    ClusterConnectionCore1ToCore0Meta& operator=(const ClusterConnectionCore1ToCore0Meta&) = delete;
    ClusterConnectionCore1ToCore0Meta& operator=(ClusterConnectionCore1ToCore0Meta&&) = delete;
    ClusterConnectionCore1ToCore0Meta(ClusterConnectionCore1ToCore0Meta&&) = delete;

    [[nodiscard]] uint8_t getSourceClusterId() const final { return etl::to_underlying(core::ClusterId::Core1); }

    [[nodiscard]] uint8_t getTargetClusterId() const final { return etl::to_underlying(core::ClusterId::Core0); }

    [[nodiscard]] bool write(const core::Message& msg) const final
    {
        etl::remove_pointer<decltype(::middleware::shm::getQueueToCore0())>::type::Sender sender(*::middleware::shm::getQueueToCore0());
        return sender.write(msg);
    }

    [[nodiscard]] size_t registeredTransceiversCount(uint16_t serviceId) const final
    {
        return core::meta::DbManipulator::registeredTransceiversCount(
            proxyTransceivers_.begin(), proxyTransceivers_.end(), serviceId);
    }

    [[nodiscard]] core::HRESULT dispatchMessage(const core::Message& msg) const final
    {
        return IClusterConnectionConfigurationBase::dispatchMessageToProxy(
            proxyTransceivers_.begin(), proxyTransceivers_.end(), msg);
    }

    [[nodiscard]] core::HRESULT subscribe(core::ProxyBase& proxy, uint16_t serviceInstanceId) final
    {
        return core::meta::DbManipulator::subscribe(
            proxyTransceivers_.begin(), proxyTransceivers_.end(), proxy, serviceInstanceId);
    }

    void unsubscribe(core::ProxyBase& proxy, uint16_t serviceId) final
    {
        core::meta::DbManipulator::unsubscribe(
            proxyTransceivers_.begin(), proxyTransceivers_.end(), proxy, serviceId);
    }

    ~ClusterConnectionCore1ToCore0Meta() = default;

  private:
    etl::vector<core::TransceiverBase*, 1U> orgtestfooFooProxyTransceivers_;
    etl::array<core::meta::TransceiverContainer, 1U> proxyTransceivers_;
};

}  // namespace middleware::meta
