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

#include "etl/string.h"
#include "someip/ISomeIpSerializable.h"

class TimeServiceReply : public someip::ISomeIpSerializable
{
public:
    enum : uint8_t
    {
        TS_PAYLOAD_LENGTH = 26U
    };

    etl::string<TS_PAYLOAD_LENGTH> _timeStr;

    TimeServiceReply() = default;

    TimeServiceReply(TimeServiceReply const& other) : _timeStr(other._timeStr) {}

    void serializeToArray(someip::SomeIpSerializer& serializer) const override;
    void parseFromArray(someip::SomeIpParser& parser) override;
    uint32_t getSize() const override;
};
