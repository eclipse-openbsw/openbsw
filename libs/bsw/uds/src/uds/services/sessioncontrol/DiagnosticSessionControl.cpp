// Copyright 2024 Accenture.

#include "uds/services/sessioncontrol/DiagnosticSessionControl.h"

#include "uds/DiagDispatcher.h"
#include "uds/UdsConfig.h"
#include "uds/UdsLogger.h"
#include "uds/connection/IncomingDiagConnection.h"
#include "uds/services/sessioncontrol/ISessionPersistence.h"
#include "uds/session/ApplicationDefaultSession.h"
#include "uds/session/ApplicationExtendedSession.h"
#include "uds/session/DiagSession.h"
#include "uds/session/IDiagSessionManager.h"
#include "uds/session/ProgrammingSession.h"

#include <async/Async.h>

#include <platform/config.h>

namespace uds
{
using ::util::logger::Logger;
using ::util::logger::UDS;

DiagnosticSessionControl::DiagnosticSessionControl(
    IUdsLifecycleConnector& udsLifecycleConnector,
    ::async::ContextType context,
    ISessionPersistence& persistence)
: Service(ServiceId::DIAGNOSTIC_SESSION_CONTROL, DiagSession::ALL_SESSIONS())
, fInitCompleteDelegate()
, fContext(context)
, fUdsLifecycleConnector(udsLifecycleConnector)
, fPersistence(persistence)
, diagDispatcher(nullptr)
, fpCurrentSession(&DiagSession::APPLICATION_DEFAULT_SESSION())
, fpRequestedJob(nullptr)
, fpTesterPresent(nullptr)
, fpReadDataByIdentifier(nullptr)
, fpEcuReset(nullptr)
, fpDiagnosticSessionControl(nullptr)
, fNextSessionTimeout(UdsVmsConstants::TESTER_PRESENT_TIMEOUT_MS)
, fInitializing(false)
, fRequestApplicationSession(false)
, fRequestProgrammingSession(false)
, fTesterPresentReceived(false)
, fIsActive(false)
, fListeners()
{
    enableSuppressPositiveResponse();
}

void DiagnosticSessionControl::init(
    AbstractDiagJob& readDataByIdentifier,
    AbstractDiagJob& ecuReset,
    AbstractDiagJob& diagnosticSessionControl,
    AbstractDiagJob& testerPresent,
    InitCompleteCallbackType const initializedDelegate)
{
    fpRequestedJob        = nullptr;
    fInitCompleteDelegate = initializedDelegate;

    fpTesterPresent            = &testerPresent;
    fpDiagnosticSessionControl = &diagnosticSessionControl;
    fpReadDataByIdentifier     = &readDataByIdentifier;
    fpEcuReset                 = &ecuReset;
    fInitializing              = true;
    fPersistence.readSession(*this);
}

void DiagnosticSessionControl::shutdown()
{
    fAsyncTimeout.cancel();
    fIsActive = false;
}

DiagSession const& DiagnosticSessionControl::getActiveSession() const { return *fpCurrentSession; }

void DiagnosticSessionControl::responseSent(
    IncomingDiagConnection& connection, ResponseSendResult const /* result */)
{
    connection.terminate();
    if (fRequestProgrammingSession)
    {
        uint8_t const session      = static_cast<uint8_t>(DiagSession::PROGRAMMING);
        fRequestProgrammingSession = false;
        fPersistence.writeSession(*this, session);
    }
}

DiagReturnCode::Type DiagnosticSessionControl::process(
    IncomingDiagConnection& connection, uint8_t const* const request, uint16_t const requestLength)
{
    if ((fpCurrentSession == nullptr) || (requestLength != 1U))
    {
        return DiagReturnCode::ISO_INVALID_FORMAT;
    }
    DiagSession::SessionType const requestedSession
        = static_cast<DiagSession::SessionType>(request[0]);
    Logger::debug(UDS, "%d -> %d", fpCurrentSession->getType(), requestedSession);

    if (fpCurrentSession == nullptr)
    {
        return DiagReturnCode::ISO_GENERAL_PROGRAMMING_FAILURE;
    }
    // each session object needs to know if it is part of the bootloader or the application (or
    // both). If a subfunction that is supported in application is not supported at all in boot,
    // then NRC12 must be returned. If a subfunction is supported in the current "flashable entity",
    // but the jump is never allowed, then NRC 7E must be returned. Finally, if the jump from the
    // current session to the new session is allowed, but there are some extra conditions that are
    // not met, then NRC22 must be returned.
    // All of these checks must be implemented by each diagnostic session object.
    DiagReturnCode::Type const switchSessionResult
        = fpCurrentSession->isTransitionPossible(requestedSession);
    if (DiagReturnCode::OK != switchSessionResult)
    {
        return switchSessionResult;
    }

    connection.addIdentifier();
    PositiveResponse& response = connection.releaseRequestGetResponse();
    if (response.getMaximumLength() < 4U)
    {
        return DiagReturnCode::ISO_RESPONSE_TOO_LONG;
    }

    if (requestedSession == DiagSession::PROGRAMMING)
    {
        (void)response.appendUint16(DEFAULT_DIAG_RESPONSE_TIME);
        (void)response.appendUint16(EXTENDED_DIAG_RESPONSE_PENDING);
        Logger::debug(UDS, "EXTENDED TIMEOUTS");
    }
    else
    {
        (void)response.appendUint16(DEFAULT_DIAG_RESPONSE_TIME);
        (void)response.appendUint16(DEFAULT_DIAG_RESPONSE_PENDING);
        Logger::debug(UDS, "DEFAULT SESSION TIMEOUTS");
    }

    DiagSession& newSession = fpCurrentSession->getTransitionResult(requestedSession);
    switchSession(newSession);

    (void)connection.sendPositiveResponseInternal(response.getLength(), *this);
    Logger::debug(UDS, "Active Session 0x%x", fpCurrentSession->getSessionByte());
    return DiagReturnCode::OK;
}

void DiagnosticSessionControl::startSessionTimeout() { startTimeout(); }

void DiagnosticSessionControl::stopSessionTimeout() { stopTimeout(); }

bool DiagnosticSessionControl::isSessionTimeoutActive() { return fIsActive; }

void DiagnosticSessionControl::resetToDefaultSession()
{
    DiagSession& newSession = fpCurrentSession->getTransitionResult(DiagSession::DEFAULT);
    switchSession(newSession);
}

DiagReturnCode::Type DiagnosticSessionControl::acceptedJob(
    IncomingDiagConnection& connection,
    AbstractDiagJob const& job,
    uint8_t const* const request,
    uint16_t const requestLength)
{
    return this->acceptedJob_ext(connection, job, request, requestLength);
}

DiagReturnCode::Type DiagnosticSessionControl::acceptedJob_ext(
    IncomingDiagConnection const& /* connection */,
    AbstractDiagJob const& job,
    uint8_t const* const /* request */,
    uint16_t const /* requestLength */)
{
    Logger::debug(
        UDS,
        "Accepted 0x%x, current session: 0x%x",
        job.getRequestId(),
        getActiveSession().getSessionByte());

    fTesterPresentReceived = (&job == fpTesterPresent);
    stopSessionTimeout();
    return DiagReturnCode::OK;
}

ESR_NO_INLINE void DiagnosticSessionControl::responseSent_local(
    IncomingDiagConnection const& /*connection*/,
    DiagReturnCode::Type const result,
    uint8_t const* const /*response*/,
    uint16_t const /*responseLength*/)
{
    Logger::debug(UDS, "Sent response 0x%x, tp %d", result, fTesterPresentReceived);
    if (result != DiagReturnCode::ISO_RESPONSE_PENDING)
    {
        // any response except pending restarts the session timeout
        startSessionTimeout();
    }

    if (fTesterPresentReceived)
    { // do nothing
        fTesterPresentReceived = false;
    }
    else
    { // notify
        for (auto itr = fListeners.begin(); itr != fListeners.end(); ++itr)
        {
            itr->diagSessionResponseSent(static_cast<uint8_t>(result));
        }
    }
}

void DiagnosticSessionControl::responseSent(
    IncomingDiagConnection& connection,
    DiagReturnCode::Type const result,
    uint8_t const* const response,
    uint16_t const responseLength)
{
    responseSent_local(connection, result, response, responseLength);
}

ESR_NO_INLINE void
DiagnosticSessionControl::addDiagSessionListener_local(IDiagSessionChangedListener& listener)
{
    ::async::LockType const lock;
    fListeners.push_back(listener);
}

void DiagnosticSessionControl::addDiagSessionListener(IDiagSessionChangedListener& listener)
{
    addDiagSessionListener_local(listener);
}

ESR_NO_INLINE void
DiagnosticSessionControl::removeDiagSessionListener_local(IDiagSessionChangedListener& listener)
{
    ::async::LockType const lock;
    fListeners.erase(listener);
}

void DiagnosticSessionControl::removeDiagSessionListener(IDiagSessionChangedListener& listener)
{
    removeDiagSessionListener_local(listener);
}

ESR_NO_INLINE void DiagnosticSessionControl::expired_local()
{
    Logger::warn(UDS, "Session timeout in session 0x%x", fpCurrentSession->getSessionByte());

    if ((*fpCurrentSession == DiagSession::APPLICATION_EXTENDED_SESSION()))

    {
        switchSession(DiagSession::APPLICATION_DEFAULT_SESSION());
    }
    else
    {
        Logger::error(
            UDS,
            "Session timeout in session 0x%x is NOT allowed!",
            fpCurrentSession->getSessionByte());
    }
}

void DiagnosticSessionControl::execute() { expired_local(); }

void DiagnosticSessionControl::setTimeout(uint32_t const timeout)
{
    fAsyncTimeout.cancel();
    fIsActive = true;
    ::async::schedule(fContext, *this, fAsyncTimeout, timeout, ::async::TimeUnit::MILLISECONDS);
}

void DiagnosticSessionControl::startTimeout()
{
    if ((*fpCurrentSession == DiagSession::APPLICATION_EXTENDED_SESSION())
        || (*fpCurrentSession == DiagSession::PROGRAMMING_SESSION()))
    {
        setTimeout(fNextSessionTimeout);
        fNextSessionTimeout = UdsVmsConstants::TESTER_PRESENT_TIMEOUT_MS;
    }
}

void DiagnosticSessionControl::stopTimeout()
{
    fAsyncTimeout.cancel();
    fIsActive = true;
}

void DiagnosticSessionControl::switchSession(DiagSession& newSession)
{
    uint8_t const oldSessionByte = fpCurrentSession->getSessionByte();
    if (newSession == DiagSession::APPLICATION_DEFAULT_SESSION())
    {
        fpCurrentSession = &newSession;
        fpCurrentSession->enter();
        stopTimeout();
    }
    else if (newSession == DiagSession::APPLICATION_EXTENDED_SESSION())
    {
        if (fpCurrentSession != &newSession)
        {
            fpCurrentSession = &newSession;
            fpCurrentSession->enter();
        }
        setTimeout(UdsVmsConstants::TESTER_PRESENT_TIMEOUT_MS);
    }
    else if (newSession == DiagSession::PROGRAMMING_SESSION())
    {
        if (diagDispatcher != nullptr)
        {
            diagDispatcher->fEnabled = false;
        }
        fRequestProgrammingSession = true;
    }
    else
    {
        fpCurrentSession = &newSession;
        fpCurrentSession->enter();
    }
    Logger::debug(
        UDS, "switching from session 0x%x to 0x%x ", oldSessionByte, newSession.getSessionByte());

    for (auto itr = fListeners.begin(); itr != fListeners.end(); ++itr)
    {
        itr->diagSessionChanged(*fpCurrentSession);
    }
}

/* sessionRead/sessionWritten callback for persistence :
 *   - read sessionstate so far not propagated
 *   - written sessionstate called before entering programming
 *     which requires a reset
 *
 */
void DiagnosticSessionControl::sessionRead(uint8_t const)
{
    Logger::debug(UDS, "Starting up with Session = 0x%x", fpCurrentSession->getSessionByte());

    switchSession(DiagSession::APPLICATION_DEFAULT_SESSION());
    if (fInitCompleteDelegate.is_valid())
    {
        fInitCompleteDelegate();
    }
    fInitializing = false;
}

void DiagnosticSessionControl::sessionWritten(bool const successful)
{
    if (!successful)
    {
        Logger::error(UDS, "Writing session to eeprom failed!");
    }
    else
    {
        (void)fUdsLifecycleConnector.requestShutdown(
            IUdsLifecycleConnector::HARD_RESET, RESET_TIME);
    }
}

} // namespace uds
