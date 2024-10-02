// Copyright 2024 Accenture.

#ifndef GUARD_899F066E_9209_43DF_B727_D4C75179C021
#define GUARD_899F066E_9209_43DF_B727_D4C75179C021

#include <estd/forward_list.h>
#include <estd/uncopyable.h>

namespace uds
{
class ICommunicationStateListener : public ::estd::forward_list_node<ICommunicationStateListener>
{
public:
    enum CommunicationState
    {
        ENABLE_NORMAL_MESSAGE_TRANSMISSION,
        DISABLE_NORMAL_MESSAGE_TRANSMISSION,
        ENABLE_REC_DISABLE_NORMAL_MESSAGE_SEND_TRANSMISSION,
        ENABLE_NN_MESSAGE_TRANSMISSION,
        DISABLE_NM_MESSAGE_TRANSMISSION,
        ENABLE_REC_DISABLE_NM_SEND_TRANSMISSION,
        ENABLE_ALL_MESSAGE_TRANSMISSION,
        DISABLE_ALL_MESSAGE_TRANSMISSION,
        ENABLE_REC_DISABLE_ALL_SEND_TRANSMISSION,
        DISABLE_REC_ENABLE_NORMAL_MESSAGE_SEND_TRANSMISSION,
        DISABLE_REC_ENABLE_NM_SEND_TRANSMISSION,
        DISABLE_REC_ENABLE_ALL_SEND_TRANSMISSION
    };

    ICommunicationStateListener() {}

    virtual void communicationStateChanged(CommunicationState newState) = 0;

private:
    UNCOPYABLE(ICommunicationStateListener);
};

} // namespace uds

#endif // GUARD_899F066E_9209_43DF_B727_D4C75179C021
