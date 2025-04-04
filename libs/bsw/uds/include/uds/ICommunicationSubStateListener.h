// Copyright 2024 Accenture.

#pragma once

#include <estd/forward_list.h>
#include <estd/uncopyable.h>

namespace uds
{
/**
 * listener class to derive from in case the communication
 * control is aimed for sub-bus segment to diagnostic-only
 * scheduling mode. see the iso 14229-1 for more information.
 */
class ICommunicationSubStateListener
: public ::estd::forward_list_node<ICommunicationSubStateListener>
{
public:
    enum CommunicationEnhancedState
    {
        ENABLE_REC_DISABLE_ENHANCED_SEND_TRANSMISSION,
        ENABLE_ENHANCED_TRANSMISSION
    };

    ICommunicationSubStateListener() {}

    /**
     * when service CommunicationControl is called with EnhancedAddressInformation controlType
     * all registered substate listener will be called.
     *
     * \param newState new state to be set
     * \param nodeId identifies the node in the subnetwork
     * \return true in case subnetwork node is known, false otherwise.
     */
    virtual bool communicationStateChanged(CommunicationEnhancedState newState, uint16_t /*nodeId*/)
        = 0;

private:
    UNCOPYABLE(ICommunicationSubStateListener);
};

} // namespace uds
