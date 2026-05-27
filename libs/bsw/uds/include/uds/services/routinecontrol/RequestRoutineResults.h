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

namespace uds
{
/**
 * UDS subfunction RequestRoutineResults (0x31, 0x03)
 *
 */
class RequestRoutineResults : public Subfunction
{
public:
    RequestRoutineResults();

private:
    static uint8_t const sfImplementedRequest[2];
};

} // namespace uds
