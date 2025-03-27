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
    MOCK_METHOD6(
        getTransportMessage,
        ErrorCode(
            uint8_t,
            uint16_t,
            uint16_t,
            uint16_t,
            ::estd::slice<uint8_t const> const&,
            TransportMessage*&));

    MOCK_METHOD1(releaseTransportMessage, void(TransportMessage&));

    MOCK_METHOD0(dump, void());
};

} // namespace transport
