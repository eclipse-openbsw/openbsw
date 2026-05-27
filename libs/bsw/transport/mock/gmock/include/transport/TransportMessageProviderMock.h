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

#include "transport/ITransportMessageProvider.h"

#include <etl/span.h>

#include <gmock/gmock.h>

namespace transport
{
class TransportMessageProviderMock : public ITransportMessageProvider
{
public:
    MOCK_METHOD(
        ErrorCode,
        getTransportMessage,
        (uint8_t,
         uint16_t,
         uint16_t,
         uint16_t,
         ::etl::span<uint8_t const> const&,
         TransportMessage*&));

    MOCK_METHOD(void, releaseTransportMessage, (TransportMessage&));

    MOCK_METHOD(void, dump, ());
};

} // namespace transport
