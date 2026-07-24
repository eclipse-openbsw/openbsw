/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "uds/base/Subfunction.h"

#include <etl/array.h>

namespace uds
{
/**
 * UDS subfunction StartRoutine (0x31, 0x01)
 *
 */
class StartRoutine : public Subfunction
{
public:
    StartRoutine();

private:
    static ::etl::array<uint8_t, 2U> const sfImplementedRequest;
};

} // namespace uds
