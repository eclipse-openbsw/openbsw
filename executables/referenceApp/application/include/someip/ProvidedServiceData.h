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

#include <cstdint>

class ProvidedServiceData : public someip::ISomeIpSerializable
{
public:
    uint32_t data;

    void serializeToArray(someip::SomeIpSerializer& serializer) const override;
    void parseFromArray(someip::SomeIpParser& parser) override;
    uint32_t getSize() const override;
};
