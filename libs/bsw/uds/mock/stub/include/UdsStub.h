// Copyright 2024 Accenture.

#pragma once

#include "uds/IDiagDispatcher.h"
#include "uds/base/AbstractDiagJob.h"
#include "uds/session/ApplicationDefaultSession.h"
#include "uds/session/ApplicationExtendedSession.h"
#include "uds/session/IDiagSessionChangedListener.h"
#include "uds/session/IDiagSessionManager.h"

#ifndef DCM_E_POSITIVERESPONSE
#define DCM_E_POSITIVERESPONSE 0U
#endif /* DCM_E_POSITIVERESPONSE */

namespace transport
{
class TransportMessage;
}

namespace uds
{

class IDiagAuthenticator
{};

class TestDiagnosisSessionManager : public IDiagSessionManager
{
public:
    TestDiagnosisSessionManager() : fpCurrentSession(&DiagSession::APPLICATION_DEFAULT_SESSION()) {}

    /* DSI: test function*/
    void testSwitchSession(DiagSession::SessionType targetSession);

    virtual DiagSession const& getActiveSession() const { return *fpCurrentSession; }

    virtual void startSessionTimeout() {}

    virtual void stopSessionTimeout() {}

    virtual bool isSessionTimeoutActive() { return false; }

    virtual void resetToDefaultSession()
    {
        this->testSwitchSession(DiagSession::DEFAULT);
        for (::estd::forward_list<IDiagSessionChangedListener>::iterator itr = fListeners.begin();
             itr != fListeners.end();
             ++itr)
        {
            itr->diagSessionChanged(*fpCurrentSession);
        }
    }

    virtual DiagReturnCode::Type acceptedJob(
        IncomingDiagConnection& connection,
        AbstractDiagJob const& job,
        uint8_t const request[],
        uint16_t requestLength)
    {
        return DiagReturnCode::OK;
    }

    virtual void responseSent(
        IncomingDiagConnection& connection,
        DiagReturnCode::Type result,
        uint8_t const response[],
        uint16_t responseLength);

    virtual void addDiagSessionListener(IDiagSessionChangedListener&);

    virtual void removeDiagSessionListener(IDiagSessionChangedListener&);

private:
    DiagSession* fpCurrentSession;
    ::estd::forward_list<IDiagSessionChangedListener> fListeners;
};

class DiagDispatcher : public IDiagDispatcher
{
private:
    DiagJobRoot fDiagJobRoot;

public:
    DiagDispatcher(IDiagSessionManager& sessionManager)
    : IDiagDispatcher(sessionManager, fDiagJobRoot)
    {}

    virtual uint16_t getSourceId() const { return 0xab; }

    virtual uint8_t dispatchTriggerEventRequest(transport::TransportMessage& msg) { return 0; }

    virtual IOutgoingDiagConnectionProvider::ErrorCode getOutgoingDiagConnection(
        uint16_t targetId,
        OutgoingDiagConnection*& pConnection,
        transport::TransportMessage* pRequestMessage)
    {
        return CONNECTION_OK;
    }
};

} // namespace uds

