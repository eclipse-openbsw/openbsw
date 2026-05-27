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

#include "uds/resume/IResumableResetDriverPersistence.h"

#include <gmock/gmock.h>

namespace uds
{
class ResumableResetDriverPersistenceMock : public IResumableResetDriverPersistence
{
public:
    MOCK_METHOD(bool, readRequest, (::transport::TransportMessage & message), (const, override));
    MOCK_METHOD(void, writeRequest, (::transport::TransportMessage const& message), (override));
    MOCK_METHOD(void, clear, (), (override));
};
} // namespace uds
