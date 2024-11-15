// Copyright 2024 Accenture.

#include "uds/services/communicationcontrol/CommunicationControl.h"

#include "uds/UdsLogger.h"
#include "uds/connection/IncomingDiagConnection.h"
#include "uds/session/ApplicationExtendedSession.h"
#include "uds/session/IDiagSessionManager.h"

#include <etl/unaligned_type.h>

#include <platform/config.h>

namespace uds
{
using ::util::logger::Logger;
using ::util::logger::UDS;

CommunicationControl::CommunicationControl(DiagSessionMask const sessionMask)
: Service(
    ServiceId::COMMUNICATION_CONTROL,
    AbstractDiagJob::VARIABLE_REQUEST_LENGTH,
    RESPONSE_LENGTH,
    sessionMask)
, fCommunicationState(ICommunicationStateListener::DISABLE_NORMAL_MESSAGE_TRANSMISSION)
, fSubNodeIdDisabledTx(0U)
, fNeverNotified(true)
, fListeners()
, fSubListeners()
{
    enableSuppressPositiveResponse();
}

ESR_NO_INLINE void
CommunicationControl::addCommunicationStateListener_local(ICommunicationStateListener& listener)
{
    fListeners.push_back(listener);
}

ESR_NO_INLINE void
CommunicationControl::removeCommunicationStateListener_local(ICommunicationStateListener& listener)
{
    fListeners.erase(listener);
}

ESR_NO_INLINE void CommunicationControl::addCommunicationSubStateListener_local(
    ICommunicationSubStateListener& listener)
{
    fSubListeners.push_back(listener);
}

ESR_NO_INLINE void CommunicationControl::removeCommunicationSubStateListener_local(
    ICommunicationSubStateListener& listener)
{
    fSubListeners.erase(listener);
}

void CommunicationControl::addCommunicationStateListener(ICommunicationStateListener& listener)
{
    addCommunicationStateListener_local(listener);
}

void CommunicationControl::removeCommunicationStateListener(ICommunicationStateListener& listener)
{
    removeCommunicationStateListener_local(listener);
}

void CommunicationControl::addCommunicationSubStateListener(
    ICommunicationSubStateListener& listener)
{
    addCommunicationSubStateListener_local(listener);
}

void CommunicationControl::removeCommunicationSubStateListener(
    ICommunicationSubStateListener& listener)
{
    removeCommunicationSubStateListener_local(listener);
}

ICommunicationStateListener::CommunicationState CommunicationControl::getCommunicationState() const
{
    return fCommunicationState;
}

void CommunicationControl::setCommunicationState(
    ICommunicationStateListener::CommunicationState const state)
{
    if ((fCommunicationState != state) || (fNeverNotified))
    {
        fCommunicationState = state;
        fNeverNotified      = false;
        notifyListeners();
    }
}

void CommunicationControl::resetCommunicationSubState()
{
    (void)notifySubListeners(
        fSubNodeIdDisabledTx, ICommunicationSubStateListener::ENABLE_ENHANCED_TRANSMISSION);
    fSubNodeIdDisabledTx = 0U;
}

// METRIC STCYC 25 // The method is already in use as is
DiagReturnCode::Type CommunicationControl::process(
    IncomingDiagConnection& connection, uint8_t const* const request, uint16_t const requestLength)
{
    DiagReturnCode::Type ret = sessionAccepted();
    if (DiagReturnCode::OK != ret)
    {
        return ret;
    }
    uint8_t const controlType         = request[0];
    uint8_t const communicationTypeLo = request[1] & 0x0FU;
    uint16_t rcvNode                  = 0U;

    /* if communicationTypeHi is set with value 0xF0 it means :
            Disable/Enable network which request is received on (Receiving node (server))
            REC_NODE_COMMUNICATION_MESSAGES  is the enum
       if between 0x10 .. 0xE0 it could be a NM by number.
    */

    uint8_t const communicationTypeHi = request[1U] & 0xF0U;

    if ((((static_cast<uint8_t>(ENABLE_RX_AND_DISABLE_TX_WITH_ENHANCED_ADDRESS_INFORMATION)
           == controlType)
          || (static_cast<uint8_t>(ENABLE_RX_AND_TX_WITH_ENHANCED_ADDRESS_INFORMATION)
              == controlType))
         && (requestLength != EXPECTED_REQUEST_LENGTH_EXTENDED))
        || ((controlType <= static_cast<uint8_t>(DISABLE_RX_AND_TX))
            && (requestLength != EXPECTED_REQUEST_LENGTH)))
    {
        return DiagReturnCode::ISO_INVALID_FORMAT;
    }

    PositiveResponse& response = connection.releaseRequestGetResponse();

    ICommunicationStateListener::CommunicationState newState
        = fCommunicationState; // default to no change

    /* */
    switch (controlType)
    {
        case DISABLE_RX_AND_ENABLE_TX:
        {
            if (static_cast<uint8_t>(NORMAL_COMMUNICATION_MESSAGES) == communicationTypeLo)
            {
                newState = ICommunicationStateListener::
                    DISABLE_REC_ENABLE_NORMAL_MESSAGE_SEND_TRANSMISSION;
            }
            else if (static_cast<uint8_t>(NM_COMMUNICATION_MESSAGES) == communicationTypeLo)
            {
                newState = ICommunicationStateListener::DISABLE_REC_ENABLE_NM_SEND_TRANSMISSION;
            }
            else
            {
                newState = ICommunicationStateListener::DISABLE_REC_ENABLE_ALL_SEND_TRANSMISSION;
            }
            setCommunicationState(newState);
            (void)response.appendUint8(controlType);
            (void)connection.sendPositiveResponseInternal(response.getLength(), *this);
        }
        break;
        case DISABLE_RX_AND_TX:
        {
            if (static_cast<uint8_t>(NORMAL_COMMUNICATION_MESSAGES) == communicationTypeLo)
            {
                newState = ICommunicationStateListener::DISABLE_NORMAL_MESSAGE_TRANSMISSION;
            }
            else if (static_cast<uint8_t>(NM_COMMUNICATION_MESSAGES) == communicationTypeLo)
            {
                newState = ICommunicationStateListener::DISABLE_NM_MESSAGE_TRANSMISSION;
            }
            else
            {
                newState = ICommunicationStateListener::DISABLE_ALL_MESSAGE_TRANSMISSION;
            }

            setCommunicationState(newState);
            (void)response.appendUint8(controlType);
            (void)connection.sendPositiveResponseInternal(response.getLength(), *this);
        }
        break;
        case ENABLE_RX_AND_TX:
        {
            if (static_cast<uint8_t>(NORMAL_COMMUNICATION_MESSAGES) == communicationTypeLo)
            {
                newState = ICommunicationStateListener::ENABLE_NORMAL_MESSAGE_TRANSMISSION;
            }
            else if (static_cast<uint8_t>(NM_COMMUNICATION_MESSAGES) == communicationTypeLo)
            {
                newState = ICommunicationStateListener::ENABLE_NN_MESSAGE_TRANSMISSION;
            }
            else
            {
                newState = ICommunicationStateListener::ENABLE_ALL_MESSAGE_TRANSMISSION;
            }
            setCommunicationState(newState);
            // reset also subnet nodes under these circumstances
            if (((REC_NODE_COMMUNICATION_MESSAGES == communicationTypeHi)
                 || (NO_REC_NODE_COMMUNICATION_MESSAGES == communicationTypeHi))
                && (fSubNodeIdDisabledTx != 0U))
            {
                resetCommunicationSubState();
            }

            (void)response.appendUint8(controlType);
            (void)connection.sendPositiveResponseInternal(response.getLength(), *this);
        }
        break;
        case ENABLE_RX_AND_DISABLE_TX:
        {
            if (static_cast<uint8_t>(NORMAL_COMMUNICATION_MESSAGES) == communicationTypeLo)
            {
                newState = ICommunicationStateListener::
                    ENABLE_REC_DISABLE_NORMAL_MESSAGE_SEND_TRANSMISSION;
            }
            else if (static_cast<uint8_t>(NM_COMMUNICATION_MESSAGES) == communicationTypeLo)
            {
                newState = ICommunicationStateListener::ENABLE_REC_DISABLE_NM_SEND_TRANSMISSION;
            }
            else
            {
                newState = ICommunicationStateListener::ENABLE_REC_DISABLE_ALL_SEND_TRANSMISSION;
            }
            setCommunicationState(newState);
            (void)response.appendUint8(controlType);
            (void)connection.sendPositiveResponseInternal(response.getLength(), *this);
        }
        break;
        case ENABLE_RX_AND_DISABLE_TX_WITH_ENHANCED_ADDRESS_INFORMATION:
        {
            if (static_cast<uint8_t>(NORMAL_COMMUNICATION_MESSAGES) == communicationTypeLo)
            {
                rcvNode = ::etl::be_uint16_t(&request[2]);
                if (0U == fSubNodeIdDisabledTx)
                {
                    if (notifySubListeners(
                            rcvNode,
                            static_cast<uint8_t>(
                                ICommunicationSubStateListener::
                                    ENABLE_REC_DISABLE_ENHANCED_SEND_TRANSMISSION)))
                    {
                        fSubNodeIdDisabledTx = rcvNode;
                    }
                    else
                    {
                        ret = DiagReturnCode::ISO_SUBFUNCTION_NOT_SUPPORTED;
                    }
                }
                else if (fSubNodeIdDisabledTx != rcvNode)
                {
                    ret = DiagReturnCode::ISO_SUBFUNCTION_NOT_SUPPORTED;
                }
                else
                {
                    // nothing to do
                }
            }
            if (DiagReturnCode::OK == ret)
            {
                (void)response.appendUint8(controlType);
                (void)connection.sendPositiveResponseInternal(response.getLength(), *this);
            }
        }
        break;
        case ENABLE_RX_AND_TX_WITH_ENHANCED_ADDRESS_INFORMATION:
        {
            if (static_cast<uint8_t>(NORMAL_COMMUNICATION_MESSAGES) == communicationTypeLo)
            {
                rcvNode = ::etl::be_uint16_t(&request[2]);
                if (rcvNode == fSubNodeIdDisabledTx)
                {
                    if (notifySubListeners(
                            rcvNode,
                            static_cast<uint8_t>(
                                ICommunicationSubStateListener::ENABLE_ENHANCED_TRANSMISSION)))
                    {
                        fSubNodeIdDisabledTx = 0U;
                    }
                    else
                    {
                        ret = DiagReturnCode::ISO_SUBFUNCTION_NOT_SUPPORTED;
                    }
                }
                else
                {
                    ret = DiagReturnCode::ISO_SUBFUNCTION_NOT_SUPPORTED;
                }
            }
            if (DiagReturnCode::OK == ret)
            {
                (void)response.appendUint8(controlType);
                (void)connection.sendPositiveResponseInternal(response.getLength(), *this);
            }
        }
        break;
        default:
        {
            if ((controlType <= VMS_CONTROL_TYPE_HI) && (controlType >= VMS_CONTROL_TYPE_LO))
            {
                ret = DiagReturnCode::NOT_RESPONSIBLE;
            }
            else
            {
                ret = DiagReturnCode::ISO_SUBFUNCTION_NOT_SUPPORTED;
            }
        }
        break;
    }
    return ret;
}

void CommunicationControl::notifyListeners()
{
    for (ListenerList::iterator itr = fListeners.begin(); itr != fListeners.end(); ++itr)
    {
        itr->communicationStateChanged(fCommunicationState);
    }
}

/*
 *
 */
bool CommunicationControl::notifySubListeners(uint16_t const nodeId, uint8_t const newState)
{
    bool ret = false;
    ICommunicationSubStateListener::CommunicationEnhancedState const comState
        = static_cast<ICommunicationSubStateListener::CommunicationEnhancedState>(newState);

    for (SubListenerList::iterator itr = fSubListeners.begin(); itr != fSubListeners.end(); ++itr)
    {
        if (itr->communicationStateChanged(comState, nodeId))
        {
            ret = true;
        }
    }
    return ret;
}

} // namespace uds
