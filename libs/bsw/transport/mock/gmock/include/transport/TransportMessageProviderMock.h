// Copyright 2024 Accenture.

/**
 * Contains
 * \file
 * \ingroup
 */

#ifndef GUARD_FB0FFA17_25D3_4D62_BD96_956216D2EE61
#define GUARD_FB0FFA17_25D3_4D62_BD96_956216D2EE61

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

#endif /* GUARD_FB0FFA17_25D3_4D62_BD96_956216D2EE61 */
