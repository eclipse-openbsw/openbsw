/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "someip/ISomeIpSerializable.h"
#include "someip/SomeIpParser.h"
#include "someip/SomeIpSerializer.h"

#include <gmock/gmock.h>

namespace someip
{
class SomeIpSerializableMock : public ISomeIpSerializable
{
public:
    void serializeToArrayWithError(SomeIpSerializer& serializer) const { serializer.setFailure(); }

    MOCK_METHOD(void, serializeToArray, (SomeIpSerializer & serializer), (const));

    MOCK_METHOD(void, parseFromArray, (SomeIpParser & parser));

    MOCK_METHOD(uint32_t, getSize, (), (const));
};

} // namespace someip
