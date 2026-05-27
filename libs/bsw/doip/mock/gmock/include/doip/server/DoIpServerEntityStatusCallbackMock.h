/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

/**
 * \ingroup doip
 */
#pragma once

#include "doip/server/IDoIpServerEntityStatusCallback.h"

#include <gmock/gmock.h>

namespace doip
{
/**
 * Interface for retrieving information about entity status.
 */
class DoIpServerEntityStatusCallbackMock : public IDoIpServerEntityStatusCallback
{
public:
    MOCK_CONST_METHOD1(getEntityStatus, EntityStatus(uint8_t socketGroupId));
};

} // namespace doip
