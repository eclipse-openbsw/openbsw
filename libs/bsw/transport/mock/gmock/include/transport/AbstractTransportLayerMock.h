// Copyright 2024 Accenture.

/**
 * Contains
 * \file
 * \ingroup
 */

#ifndef GUARD_027E686A_FE36_47F0_BB6C_F48FC60383EE
#define GUARD_027E686A_FE36_47F0_BB6C_F48FC60383EE

#include "transport/AbstractTransportLayer.h"

#include <gmock/gmock.h>

namespace transport
{
class AbstractTransportLayerMock : public AbstractTransportLayer
{
public:
    AbstractTransportLayerMock(uint8_t busId) : AbstractTransportLayer(busId) {}

    virtual ~AbstractTransportLayerMock() {}

    using AbstractTransportLayer::shutdownCompleteDummy;

    MOCK_METHOD0(init, ErrorCode());
    MOCK_METHOD1(shutdown, bool(ShutdownDelegate));
    MOCK_METHOD2(send, ErrorCode(TransportMessage&, ITransportMessageProcessedListener*));

    ITransportMessageProvidingListener& getProvidingListenerHelper_impl()
    {
        return getProvidingListenerHelper();
    }
};

} // namespace transport

#endif /* GUARD_027E686A_FE36_47F0_BB6C_F48FC60383EE */
