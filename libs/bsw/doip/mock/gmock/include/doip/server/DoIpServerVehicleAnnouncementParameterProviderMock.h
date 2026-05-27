/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "doip/server/IDoIpServerVehicleAnnouncementParameterProvider.h"

#include <gmock/gmock.h>

namespace doip
{
/**
 * Mock class for vehicle identification parameters provider
 */
class DoIpServerVehicleAnnouncementParameterProviderMock
: public IDoIpServerVehicleAnnouncementParameterProvider
{
public:
    MOCK_CONST_METHOD0(getAnnounceWait, uint16_t(void));
    MOCK_CONST_METHOD0(getAnnounceInterval, uint16_t(void));
};

} // namespace doip
