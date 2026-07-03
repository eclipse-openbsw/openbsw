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
#include <etl/span.h>
#include <etl/vector.h>

#include "middleware/ClusterId.h"
#include "middleware/core/ITimeoutHandler.h"
#include "middleware/core/ProxyAttribute.h"
#include "middleware/core/ProxyBase.h"
#include "middleware/core/ProxyEventBase.h"
#include "middleware/core/ResponseBuffer.h"
#include "middleware/core/SkeletonAttribute.h"
#include "middleware/core/SkeletonBase.h"
#include "middleware/core/SkeletonEvent.h"
#include "shm/AllocatorsDefinitions.h"

namespace org::test::foo::Foo
{


struct FooStruct
{
    uint32_t fooValue;
};

namespace internal
{
constexpr uint16_t SERVICE_ID = 1U;


constexpr uint16_t get_FooDefault_id = 1U;
constexpr uint16_t set_FooDefault_id = 2U;
constexpr uint16_t event_FooDefault_id = 128U;



enum InstanceId : uint8_t
{
    InstanceId_1 = 1U,
};

::etl::span<const middleware::core::IInstanceDatabase* const> getClusterConnectionsDb();
}  // namespace internal

}  // namespace org::test::foo::Foo
