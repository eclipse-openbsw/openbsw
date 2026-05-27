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

#include "util/stream/ISharedOutputStream.h"

#include <gmock/gmock.h>

namespace util
{
namespace stream
{
class SharedOutputStreamMock : public ISharedOutputStream
{
public:
    MOCK_METHOD(IOutputStream&, startOutput, (IContinuousUser * user), (override);
    MOCK_METHOD(void, endOutput, (IContinuousUser * user), (override));
    MOCK_METHOD(void, releaseContinuousUser, (IContinuousUser & user), (override));
};

class ContinuousUserMock : public ISharedOutputStream::IContinuousUser
{
public:
    MOCK_METHOD(void, endContinuousOutput, (IOutputStream & stream), (override));
};

} // namespace stream
} // namespace util
