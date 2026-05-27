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

#include "udp/IDataListener.h"

#include <gmock/gmock.h>

namespace udp
{
struct DataListenerMock : public IDataListener
{
    MOCK_METHOD(
        void,
        dataReceived,
        (AbstractDatagramSocket&, ::ip::IPAddress, uint16_t, ::ip::IPAddress, uint16_t));
};

} // namespace udp
