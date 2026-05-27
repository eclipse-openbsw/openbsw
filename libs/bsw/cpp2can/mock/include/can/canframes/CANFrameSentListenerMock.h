/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

/**
 * Contains
 * \file
 * \ingroup
 */

#pragma once

#include "can/canframes/ICANFrameSentListener.h"

#include <gmock/gmock.h>

namespace can
{
struct CANFrameSentListenerMock : public ICANFrameSentListener
{
    MOCK_METHOD(void, canFrameSent, (CANFrame const&), (override));
};

} // namespace can
