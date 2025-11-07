// Copyright 2024 Accenture.

#pragma once

#include "uds/IDiagDispatcher.h"
#include "uds/base/AbstractDiagJob.h"
#include "uds/session/ApplicationDefaultSession.h"
#include "uds/session/ApplicationExtendedSession.h"
#include "uds/session/IDiagSessionChangedListener.h"
#include "uds/session/IDiagSessionManager.h"

#include <etl/intrusive_forward_list.h>

#ifndef DCM_E_POSITIVERESPONSE
#define DCM_E_POSITIVERESPONSE 0U
#endif /* DCM_E_POSITIVERESPONSE */

namespace transport
{
class TransportMessage;
class ITransportMessageProcessedListener;
} // namespace transport

namespace uds
{

class IDiagAuthenticator
{};

class DiagDispatcher : public IDiagDispatcher
{
private:
    DiagJobRoot fDiagJobRoot;

public:
    DiagDispatcher(IDiagSessionManager& sessionManager)
    : IDiagDispatcher(sessionManager, fDiagJobRoot)
    {}

    virtual transport::AbstractTransportLayer::ErrorCode
    resume(transport::TransportMessage&, transport::ITransportMessageProcessedListener*)
    {
        return transport::AbstractTransportLayer::ErrorCode::TP_OK;
    }

    virtual IOutgoingDiagConnectionProvider::ErrorCode getOutgoingDiagConnection(
        uint16_t targetAddress,
        OutgoingDiagConnection*& pConnection,
        transport::TransportMessage* pRequestMessage)
    {
        return CONNECTION_OK;
    }
};

} // namespace uds
