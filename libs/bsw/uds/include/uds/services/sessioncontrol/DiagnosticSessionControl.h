// Copyright 2024 Accenture.

#pragma once

#include "uds/DiagCodes.h"
#include "uds/base/Service.h"
#include "uds/lifecycle/IUdsLifecycleConnector.h"
#include "uds/session/DiagSession.h"
#include "uds/session/IDiagSessionChangedListener.h"
#include "uds/session/IDiagSessionManager.h"

#include <async/Async.h>

#include <estd/functional.h>

namespace uds
{
class IOperatingModeManager;
class ISessionPersistence;
class DiagDispatcher2;

using InitCompleteCallbackType = ::estd::function<void()>;

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
     * If a DiagDispatcher2 is set, it will be disabled when a session change
     * that requires a reset is performed.
     * \param pDiagDispatcher   DiagDispatcher2 to disable
     */
    void setDiagDispatcher(DiagDispatcher2* const pDiagDispatcher)
    {
        fpDiagDispatcher = pDiagDispatcher;
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

    static uint16_t const RESET_TIME                      = 1000U; // ms
    /**
     * This is an extended timeout which is used when the programming
     * session in bootloader is entered. This is due to the fact that
     * a tester that is connected via ethernet may need more time
     * to reconnect (e.g. slow DHCP server)
     *
     * 60s no longer needed, changed to required 10s
     */
    static uint32_t const TESTER_PRESENT_EXTENDED_TIMEOUT = 10000U; // ms
    static uint16_t const DEFAULT_DIAG_RESPONSE_TIME      = 50U;    // ms
    static uint16_t const DEFAULT_DIAG_RESPONSE_PENDING   = 500U;   // 10ms
    static uint16_t const EXTENDED_DIAG_RESPONSE_PENDING  = 3000U;  // 10ms

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
    DiagDispatcher2* fpDiagDispatcher;

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

    ::estd::forward_list<IDiagSessionChangedListener> fListeners;
};

} // namespace uds
