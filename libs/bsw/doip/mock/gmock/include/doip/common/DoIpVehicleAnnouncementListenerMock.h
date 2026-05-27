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

#include "doip/common/IDoIpVehicleAnnouncementListener.h"

#include <gmock/gmock.h>

namespace doip
{
/**
 * DoIP vehicle announcement listener mock.
 */
class DoIpVehicleAnnouncementListenerMock : public IDoIpVehicleAnnouncementListener
{
public:
    MOCK_METHOD3(
        vehicleAnnouncementReceived,
        void(uint16_t, ::ip::IPEndpoint const&, ::ip::IPEndpoint const&));
};

} // namespace doip
