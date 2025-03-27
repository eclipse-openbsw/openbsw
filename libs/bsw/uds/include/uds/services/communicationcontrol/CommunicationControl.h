// Copyright 2024 Accenture.

#pragma once

#include "uds/base/Service.h"
#include "uds/services/communicationcontrol/ICommunicationStateManager.h"
#include "uds/session/DiagSession.h"

namespace uds
{
/**
 *
 */
class CommunicationControl
: public Service
, public ICommunicationStateManager
{
public:
    /**
     * ControlType is the first byte of the request.
     */
    enum ControlType
    {
        ENABLE_RX_AND_TX                                           = 0x00,
        ENABLE_RX_AND_DISABLE_TX                                   = 0x01,
        DISABLE_RX_AND_ENABLE_TX                                   = 0x02,
        DISABLE_RX_AND_TX                                          = 0x03,
        ENABLE_RX_AND_DISABLE_TX_WITH_ENHANCED_ADDRESS_INFORMATION = 0x04,
        ENABLE_RX_AND_TX_WITH_ENHANCED_ADDRESS_INFORMATION         = 0x05
    };

    /**
     * CommunicationType is the second byte of the request bit-ored bit0..3 and 4..7
     */
    enum CommunicationLoType
    {
        NORMAL_COMMUNICATION_MESSAGES = 0x01,
        NM_COMMUNICATION_MESSAGES     = 0x02,
        ALL_COMMUNICATION_MESSAGES    = 0x03
    };

    enum CommunicationUpType
    {
        NO_REC_NODE_COMMUNICATION_MESSAGES = 0x00,
        REC_NODE_COMMUNICATION_MESSAGES    = 0xF0
    };

    explicit CommunicationControl(
        DiagSession::DiagSessionMask sessionMask
        = DiagSession::APPLICATION_EXTENDED_SESSION_MASK());

    // interface ICommunicationStateManager
    void addCommunicationStateListener(ICommunicationStateListener& listener) override;
    void removeCommunicationStateListener(ICommunicationStateListener& listener) override;
    void addCommunicationSubStateListener(ICommunicationSubStateListener& listener) override;
    void removeCommunicationSubStateListener(ICommunicationSubStateListener& listener) override;

    ICommunicationStateListener::CommunicationState getCommunicationState() const override;

    void setCommunicationState(ICommunicationStateListener::CommunicationState state) override;

    void resetCommunicationSubState() override;

protected:
    // workaround for large non virtual thunks
    void addCommunicationStateListener_local(ICommunicationStateListener& listener);
    void removeCommunicationStateListener_local(ICommunicationStateListener& listener);
    void addCommunicationSubStateListener_local(ICommunicationSubStateListener& listener);
    void removeCommunicationSubStateListener_local(ICommunicationSubStateListener& listener);

    static uint8_t const TESTER_PRESENT_NO_ANSWER         = 0x80U;
    static uint8_t const TESTER_PRESENT_ANSWER            = 0x00U;
    static uint8_t const RESPONSE_LENGTH                  = 1U;
    static uint8_t const EXPECTED_REQUEST_LENGTH_EXTENDED = 4U;
    static uint8_t const EXPECTED_REQUEST_LENGTH          = 2U;
    static uint8_t const VMS_CONTROL_TYPE_LO              = 0x40U;
    static uint8_t const VMS_CONTROL_TYPE_HI              = 0x5FU;

    DiagReturnCode::Type process(
        IncomingDiagConnection& connection,
        uint8_t const request[],
        uint16_t requestLength) override;

    virtual DiagReturnCode::Type sessionAccepted() const { return DiagReturnCode::OK; }

    void notifyListeners();
    bool notifySubListeners(uint16_t nodeId, uint8_t newState);

    ICommunicationStateListener::CommunicationState fCommunicationState;

    uint16_t fSubNodeIdDisabledTx;
    bool fNeverNotified;

    using ListenerList = ::estd::forward_list<ICommunicationStateListener>;
    ListenerList fListeners;

    using SubListenerList = ::estd::forward_list<ICommunicationSubStateListener>;
    SubListenerList fSubListeners;
};

} // namespace uds
