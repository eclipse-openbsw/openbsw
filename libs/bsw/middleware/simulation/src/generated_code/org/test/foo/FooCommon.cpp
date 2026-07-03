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
#include "org/test/foo/FooCommon.h"

#include <etl/array.h>

#include "middleware/ClusterCore0.h"
#include "middleware/ClusterCore1.h"
#include "middleware/core/InstancesDatabase.h"

namespace org::test::foo::Foo
{

namespace internal
{

class InstancesDatabase_Merged : public ::middleware::core::IInstanceDatabase
{
  public:
    constexpr InstancesDatabase_Merged() = default;
    ::etl::span<::middleware::core::IClusterConnection* const> getSkeletonConnectionsRange() const override
    {
        return ::etl::make_span(fSkeletonConnections);
    }

    ::etl::span<::middleware::core::IClusterConnection* const> getProxyConnectionsRange() const override
    {
        return ::etl::make_span(fProxyConnections);
    }

    ::etl::span<const uint16_t> getInstanceIdsRange() const override
    {
        return ::etl::make_span(fInstanceIds);
    }

  private:
    static constexpr etl::array<const uint16_t, 1U> fInstanceIds = { 1U };
    static constexpr etl::array<::middleware::core::IClusterConnection* const, 1U> fProxyConnections = {
        {        &::middleware::ClusterConnectionCore1ToCore0        }
        };
    static constexpr etl::array<::middleware::core::IClusterConnection* const, 1U> fSkeletonConnections = {
        {        &::middleware::ClusterConnectionCore0ToCore1        }
        };
};

constexpr etl::array<::middleware::core::IClusterConnection* const, 1U> InstancesDatabase_Merged::fProxyConnections;
constexpr etl::array<::middleware::core::IClusterConnection* const, 1U> InstancesDatabase_Merged::fSkeletonConnections;
constexpr etl::array<const uint16_t, 1U> InstancesDatabase_Merged::fInstanceIds;

constexpr InstancesDatabase_Merged _InstancesDatabase_Merged;

constexpr etl::array<const ::middleware::core::IInstanceDatabase* const, 1U> instancesDatabase{
    &_InstancesDatabase_Merged
};

::etl::span<const ::middleware::core::IInstanceDatabase* const> getClusterConnectionsDb()
{
    return ::etl::make_span(instancesDatabase);
}

}  // namespace internal

}  // namespace org::test::foo::Foo
