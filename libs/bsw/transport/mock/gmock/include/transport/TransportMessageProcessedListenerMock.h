// Copyright 2024 Accenture.

#ifndef GUARD_09760B3A_1F3D_42E1_8340_6966730388FF
#define GUARD_09760B3A_1F3D_42E1_8340_6966730388FF

#include "transport/ITransportMessageProcessedListener.h"

#include <gmock/gmock.h>

namespace transport
{
class TransportMessageProcessedListenerMock : public ITransportMessageProcessedListener
{
public:
    TransportMessageProcessedListenerMock() {}

    virtual ~TransportMessageProcessedListenerMock(){};

    MOCK_METHOD2(
        transportMessageProcessed,
        void(TransportMessage& transportMessage, ProcessingResult result));
};

} // namespace transport

#endif /* GUARD_09760B3A_1F3D_42E1_8340_6966730388FF */
