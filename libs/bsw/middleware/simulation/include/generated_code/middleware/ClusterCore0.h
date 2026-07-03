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

#include <etl/delegate.h>

#include "middleware/ClusterConnectionsCore0Core1.h"
#include "middleware/core/ClusterConnection.h"

namespace middleware
{

extern core::ClusterConnectionTypeSelector<meta::ClusterConnectionCore0ToCore1Meta>::type ClusterConnectionCore0ToCore1;

void initializeCore0ClusterConnection();
void processCore0Cluster(::size_t messages = 0U, bool updateTimeouts = true);
void updateCore0ClusterTimeouts();


}  // namespace middleware
