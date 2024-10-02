// Copyright 2024 Accenture.

#ifndef GUARD_5BAEA4DC_CD59_44EA_9132_DC9D1190F65E
#define GUARD_5BAEA4DC_CD59_44EA_9132_DC9D1190F65E

#include "uds/ICommunicationStateListener.h"
#include "uds/ICommunicationSubStateListener.h"

namespace uds
{
class ICommunicationStateManager
{
public:
    virtual void addCommunicationStateListener(ICommunicationStateListener& listener)          = 0;
    virtual void removeCommunicationStateListener(ICommunicationStateListener& listener)       = 0;
    virtual void addCommunicationSubStateListener(ICommunicationSubStateListener& listener)    = 0;
    virtual void removeCommunicationSubStateListener(ICommunicationSubStateListener& listener) = 0;

    virtual ICommunicationStateListener::CommunicationState getCommunicationState() const = 0;

    virtual void setCommunicationState(ICommunicationStateListener::CommunicationState state) = 0;

    virtual void resetCommunicationSubState() = 0;
};

} // namespace uds

#endif // GUARD_5BAEA4DC_CD59_44EA_9132_DC9D1190F65E
