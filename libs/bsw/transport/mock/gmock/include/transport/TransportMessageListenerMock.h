// Copyright 2024 Accenture.

/**
 * Contains
 * \file
 * \ingroup
 */

#ifndef GUARD_F008C895_93A2_4B5F_AA4E_1A635E95B4D1
#define GUARD_F008C895_93A2_4B5F_AA4E_1A635E95B4D1

#include "transport/ITransportMessageListener.h"

#include <gmock/gmock.h>

namespace transport
{
class TransportMessageListenerMock : public ITransportMessageListener
{
public:
    MOCK_METHOD3(
        messageReceived,
        ReceiveResult(uint8_t, TransportMessage&, ITransportMessageProcessedListener*));
};

} // namespace transport

#endif /* GUARD_F008C895_93A2_4B5F_AA4E_1A635E95B4D1 */
