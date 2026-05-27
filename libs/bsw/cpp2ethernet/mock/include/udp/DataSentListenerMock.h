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

#include "udp/IDataSentListener.h"

#include <gmock/gmock.h>

namespace udp
{
struct DataSentListenerMock : public IDataSentListener
{
    MOCK_METHOD(void, dataSent, (uint8_t const[], uint16_t));
};

} // namespace udp
