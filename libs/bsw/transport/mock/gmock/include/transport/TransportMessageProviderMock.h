// Copyright 2024 Accenture.

/**
 * Contains
 * \file
 * \ingroup
 */

#pragma once

#include "transport/ITransportMessageProvider.h"

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
         ::estd::slice<uint8_t const> const&,
         TransportMessage*&),
        (override));

    MOCK_METHOD(void, releaseTransportMessage, (TransportMessage&), (override));

    MOCK_METHOD(void, dump, (), (override));
};

} // namespace transport
