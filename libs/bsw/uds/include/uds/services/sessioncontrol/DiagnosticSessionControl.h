// Copyright 2024 Accenture.
// Copyright 2026 BMW AG

#pragma once

#include "uds/DiagCodes.h"
#include "uds/base/Service.h"
#include "uds/lifecycle/IUdsLifecycleConnector.h"
#include "uds/session/DiagSession.h"
#include "uds/session/IDiagSessionChangedListener.h"
#include "uds/session/IDiagSessionManager.h"

#include <async/Async.h>
#include <etl/delegate.h>
#include <etl/intrusive_list.h>

namespace uds
{
class IOperatingModeManager;
class ISessionPersistence;
class DiagDispatcher;

using InitCompleteCallbackType = ::etl::delegate<void()>;

class DiagnosticSessionControl
: public Service
, public IDiagSessionManager
, public ::async::RunnableType
{
public:
    DiagnosticSessionControl(
        IUdsLifecycleConnector& udsLifecycleConnector,
        ::async::ContextType context,
        ISessionPersistence& persistence);

    void init(
        AbstractDiagJob& readDataByIdentifier,
        AbstractDiagJob& ecuReset,
        AbstractDiagJob& diagnosticSessionControl,
        AbstractDiagJob& testerPresent,
        InitCompleteCallbackType initializedDelegate = InitCompleteCallbackType());

    void shutdown();

    DiagSession const& getActiveSession() const override;

    void responseSent(IncomingDiagConnection& connection, ResponseSendResult result) override;

    /**
     * If a DiagDispatcher is set, it will be disabled when a session change
     * that requires a reset is performed.
     * \param pDiagDispatcher   DiagDispatcher to disable
     */
    void setDiagDispatcher(DiagDispatcher* const pDiagDispatcher)
    {
        diagDispatcher = pDiagDispatcher;
    }

    /**
     * \see IDiagSessionManager::startSessionTimeout()
     */
    void startSessionTimeout() override;

    /**
     * \see IDiagSessionManager::stopSessionTimeout()
     */
    void stopSessionTimeout() override;

    /**
     * \see IDiagSessionManager::isSessionTimeoutActive()
     */
    bool isSessionTimeoutActive() override;

    /**
     * \see IDiagSessionManager::resetToDefaultSession()
     */
    void resetToDefaultSession() override;

    DiagReturnCode::Type acceptedJob(
        IncomingDiagConnection& connection,
        AbstractDiagJob const& job,
        uint8_t const request[],
        uint16_t requestLength) override;

    void responseSent(
        IncomingDiagConnection& connection,
        DiagReturnCode::Type result,
        uint8_t const response[],
        uint16_t responseLength) override;

    void addDiagSessionListener(IDiagSessionChangedListener&) override;

    void removeDiagSessionListener(IDiagSessionChangedListener&) override;
    void execute() override;

    /**
     * \note    non-volatile memory
     */

    virtual void sessionRead(uint8_t session);

    virtual void sessionWritten(bool successful);

protected:
    // workaround for large non virtual thunks
    void responseSent_local(
        IncomingDiagConnection const& connection,
        DiagReturnCode::Type result,
        uint8_t const response[],
        uint16_t responseLength);
    void addDiagSessionListener_local(IDiagSessionChangedListener&);
    void removeDiagSessionListener_local(IDiagSessionChangedListener&);
    void expired_local();

    DiagReturnCode::Type process(
        IncomingDiagConnection& connection,
        uint8_t const request[],
        uint16_t requestLength) override;

    void setTimeout(uint32_t timeout);

    void stopTimeout();

    void startTimeout();

    DiagReturnCode::Type acceptedJob_ext(
        IncomingDiagConnection const& connection,
        AbstractDiagJob const& job,
        uint8_t const request[],
        uint16_t requestLength);

    virtual void switchSession(DiagSession& newSession);

    InitCompleteCallbackType fInitCompleteDelegate;
    ::async::ContextType fContext;
    IUdsLifecycleConnector& fUdsLifecycleConnector;
    ISessionPersistence& fPersistence;
    DiagDispatcher* diagDispatcher;

    DiagSession* fpCurrentSession;
    AbstractDiagJob const* fpRequestedJob;
    AbstractDiagJob* fpTesterPresent;
    AbstractDiagJob* fpReadDataByIdentifier;
    AbstractDiagJob* fpEcuReset;
    AbstractDiagJob* fpDiagnosticSessionControl;
    uint32_t fNextSessionTimeout;
    bool fInitializing;
    bool fRequestApplicationSession;
    bool fRequestProgrammingSession;
    bool fTesterPresentReceived;
    ::async::TimeoutType fAsyncTimeout;
    bool fIsActive;

    ::etl::intrusive_list<IDiagSessionChangedListener, etl::bidirectional_link<0>> fListeners;
};

} // namespace uds
