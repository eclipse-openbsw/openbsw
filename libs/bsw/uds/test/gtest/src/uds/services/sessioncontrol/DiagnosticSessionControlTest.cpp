// Copyright 2024 Accenture.

#include "uds/services/sessioncontrol/DiagnosticSessionControl.h"

#include "transport/TransportConfiguration.h"
#include "uds/DiagDispatcher.h"
#include "uds/UdsConfig.h"
#include "uds/connection/IncomingDiagConnection.h"
#include "uds/lifecycle/UdsLifecycleConnectorMock.h"
#include "uds/services/ecureset/ECUReset.h"
#include "uds/services/readdata/ReadDataByIdentifier.h"
#include "uds/services/sessioncontrol/SessionPersistenceMock.h"
#include "uds/services/testerpresent/TesterPresent.h"
#include "uds/session/ApplicationDefaultSession.h"
#include "uds/session/ApplicationExtendedSession.h"
#include "uds/session/DiagSessionChangedListenerMock.h"
#include "uds/session/DiagSessionManagerMock.h"
#include "uds/session/ProgrammingSession.h"
#include "util/estd/function_mock.h"

#include <async/AsyncMock.h>
#include <async/TestContext.h>
#include <etl/array.h>
#include <etl/delegate.h>

#include <gtest/gtest.h>

namespace
{
using namespace ::uds;
using namespace ::testing;

class TestableDiagnosticSessionControl : public DiagnosticSessionControl
{
public:
    TestableDiagnosticSessionControl(
        IUdsLifecycleConnector& udsLifecycleConnector,
        ::async::ContextType context,
        ISessionPersistence& persistence)
    : DiagnosticSessionControl(udsLifecycleConnector, context, persistence)
    {}

    void setRequestProgrammingSession(bool requestProgrammingSession)
    {
        DiagnosticSessionControl::fRequestProgrammingSession = requestProgrammingSession;
    }

    bool getRequestProgrammingSession()
    {
        return DiagnosticSessionControl::fRequestProgrammingSession;
    }

    DiagReturnCode::Type process(
        IncomingDiagConnection& connection,
        uint8_t const request[],
        uint16_t requestLength) override
    {
        return DiagnosticSessionControl::process(connection, request, requestLength);
    }

    void setCurrentSession(DiagSession& session)
    {
        DiagnosticSessionControl::fpCurrentSession = &session;
    }

    DiagSession* getCurrentSession() { return DiagnosticSessionControl::fpCurrentSession; }

    void setTesterPresentReceived(bool testerPresentReceived)
    {
        DiagnosticSessionControl::fTesterPresentReceived = testerPresentReceived;
    }

    /* do not implement that */
    bool persistAndRestoreSession() override { return false; }

    using DiagnosticSessionControl::RESET_TIME;
};

struct DiagnosticSessionControlTest : ::testing::Test
{
    DiagnosticSessionControlTest()
    : fUdsLifecycleConnector()
    , fSessionPersistence()
    , fDiagnosticSessionControl(fUdsLifecycleConnector, _asyncContext, fSessionPersistence)
    , fIncomingDiagConnection(::async::CONTEXT_INVALID)
    , fDiagSessionChangedListener()
    {
        fResponseMessage.init(fRequestBuffer.data(), fRequestBuffer.size());

        fIncomingDiagConnection.fpRequestMessage = &fResponseMessage;
        fIncomingDiagConnection.fSourceId        = 0x10U;

        fDiagnosticSessionControl.addDiagSessionListener(fDiagSessionChangedListener);
    }

    ~DiagnosticSessionControlTest() override
    {
        fDiagnosticSessionControl.removeDiagSessionListener(fDiagSessionChangedListener);
    }

    ::async::ContextType _asyncContext{};
    ::async::AsyncMock _asyncMock;

    StrictMock<UdsLifecycleConnectorMock> fUdsLifecycleConnector;
    StrictMock<SessionPersistenceMock> fSessionPersistence;
    TestableDiagnosticSessionControl fDiagnosticSessionControl;
    IncomingDiagConnection fIncomingDiagConnection;
    StrictMock<DiagSessionChangedListenerMock> fDiagSessionChangedListener;
    transport::TransportMessage fResponseMessage;

    ::etl::array<uint8_t, 6> fRequestBuffer;
};

/**
 * \desc
 * Calling shutdown should cancel the timeout.
 */
TEST_F(DiagnosticSessionControlTest, shutdown) { fDiagnosticSessionControl.shutdown(); }

/**
 * \desc
 * getActiveSession should returns the current active session. Calling getActiveSession directly
 * after creating a DiagnosticSessionControl object should return an ApplicationDefaultSession
 * reference.
 */
TEST_F(DiagnosticSessionControlTest, getActiveSession)
{
    EXPECT_EQ(
        &DiagSession::APPLICATION_DEFAULT_SESSION(), &fDiagnosticSessionControl.getActiveSession());
}

/**
 * \desc
 * removeDiagSessionListener should do nothing, because the given listener was not added before.
 */
TEST_F(
    DiagnosticSessionControlTest,
    removeDiagSessionListener_should_do_nothing_because_the_given_listener_was_not_added_before)
{
    StrictMock<DiagSessionChangedListenerMock> diagSessionChangedListener;
    fDiagnosticSessionControl.removeDiagSessionListener(diagSessionChangedListener);
}

/**
 * \desc
 * addDiagSessionListener should do nothing, because the given listener was already added before
 * (see SetUp() function).
 */
TEST_F(
    DiagnosticSessionControlTest,
    addDiagSessionListener_should_do_nothing_because_the_given_listener_was_already_added)
{
    fDiagnosticSessionControl.addDiagSessionListener(fDiagSessionChangedListener);
}

/**
 * \desc
 * Calling responseSent should just terminate the current incoming connection, if the
 * request programming session is false.
 */
TEST_F(DiagnosticSessionControlTest, responseSent_with_request_programming_session_false)
{
    fDiagnosticSessionControl.responseSent(
        fIncomingDiagConnection, fDiagnosticSessionControl.RESPONSE_SENT);
}

/**
 * \desc
 * Calling responseSent should terminate the current incoming connection, set request programming
 * session to false and write the session, if request programming session is true.
 */
TEST_F(DiagnosticSessionControlTest, responseSent_with_request_programming_session_true)
{
    fDiagnosticSessionControl.setRequestProgrammingSession(true);

    EXPECT_CALL(
        fSessionPersistence,
        writeSession(Ref(fDiagnosticSessionControl), DiagSession::PROGRAMMING));

    fDiagnosticSessionControl.responseSent(
        fIncomingDiagConnection, fDiagnosticSessionControl.RESPONSE_SENT);

    EXPECT_FALSE(fDiagnosticSessionControl.getRequestProgrammingSession());
}

/**
 * \desc
 * process should returns the DiagReturnCode ISO_INVALID_FORMAT, if the request is an empty request.
 */
TEST_F(DiagnosticSessionControlTest, process_should_returns_ISO_INVALID_FORMAT_if_request_is_empty)
{
    uint8_t const* request = nullptr;
    EXPECT_EQ(
        DiagReturnCode::ISO_INVALID_FORMAT,
        fDiagnosticSessionControl.process(fIncomingDiagConnection, request, sizeof(request)));
}

/**
 * \desc
 * process should returns the DiagReturnCode ISO_SUBFUNCTION_NOT_SUPPORTED, if the requestedSession
 * is invalid.
 */
TEST_F(
    DiagnosticSessionControlTest,
    process_should_returns_ISO_SUBFUNCTION_NOT_SUPPORTED_if_requestedSession_is_invalid)
{
    uint8_t const request[] = {0x04U}; // invalid requestedSession

    EXPECT_EQ(
        DiagReturnCode::ISO_SUBFUNCTION_NOT_SUPPORTED,
        fDiagnosticSessionControl.process(fIncomingDiagConnection, request, sizeof(request)));
}

/**
 * \desc
 * process should returns the DiagReturnCode ISO_CONDITIONS_NOT_CORRECT, if the requestedSession
 * (PROGRAMMING session) and the current session (DEFAULT session) are different.
 */
TEST_F(
    DiagnosticSessionControlTest,
    process_should_returns_7E_if_requestedSession_is_PROGRAMMING_session)
{
    uint8_t const request[] = {0x02U}; // PROGRAMMING session

    EXPECT_EQ(
        DiagReturnCode::ISO_SUBFUNCTION_NOT_SUPPORTED_IN_ACTIVE_SESSION,
        fDiagnosticSessionControl.process(fIncomingDiagConnection, request, sizeof(request)));
}

/**
 * \desc
 * process should returns OK, if the requestedSession and the currentSession are identically
 * (DEFAULT) and if the transition is possible.
 */
TEST_F(
    DiagnosticSessionControlTest,
    process_should_returns_OK_if_sessions_are_DEFAULT_and_transition_is_possible)
{
    uint8_t const request[] = {0x01U}; // DEFAULT session

    EXPECT_CALL(fDiagSessionChangedListener, diagSessionChanged(_));

    EXPECT_EQ(
        DiagReturnCode::OK,
        fDiagnosticSessionControl.process(fIncomingDiagConnection, request, sizeof(request)));
}

/**
 * \desc
 * process should returns OK, if the requestedSession is an EXTENDED session and if the transition
 * is possible. The current session is the same as the requested session.
 */
TEST_F(
    DiagnosticSessionControlTest,
    process_should_returns_OK_if_requestedSession_is_EXTENDED_and_transition_is_possible)
{
    fDiagnosticSessionControl.setCurrentSession(DiagSession::APPLICATION_EXTENDED_SESSION());
    uint8_t const request[] = {0x03U}; // EXTENDED session

    EXPECT_CALL(_asyncMock, schedule(_asyncContext, _, _, 5000, ::async::TimeUnit::MILLISECONDS));
    EXPECT_CALL(fDiagSessionChangedListener, diagSessionChanged(_));

    EXPECT_EQ(
        DiagReturnCode::OK,
        fDiagnosticSessionControl.process(fIncomingDiagConnection, request, sizeof(request)));
}

/**
 * \desc
 * process should returns OK, if the current session is an EXTENDED session and the transition to
 * a DEFAULT session is possible.
 */
TEST_F(
    DiagnosticSessionControlTest,
    process_should_returns_OK_if_requestedSession_is_default_session_and_current_session_is_extended)
{
    EXPECT_CALL(fDiagSessionChangedListener, diagSessionChanged(_));

    fDiagnosticSessionControl.setCurrentSession(DiagSession::APPLICATION_EXTENDED_SESSION());
    uint8_t request[] = {0x01U}; // default session
    EXPECT_EQ(
        DiagReturnCode::OK,
        fDiagnosticSessionControl.process(fIncomingDiagConnection, request, sizeof(request)));
}

/**
 * \desc
 * process should returns OK, if the current session is an EXTENDED session and the transition to
 * a PROGRAMMING session is possible.
 */
TEST_F(
    DiagnosticSessionControlTest,
    process_should_returns_OK_if_requestedSession_is_programming_session_and_current_session_is_extended)
{
    EXPECT_CALL(fDiagSessionChangedListener, diagSessionChanged(_));

    fDiagnosticSessionControl.setCurrentSession(DiagSession::APPLICATION_EXTENDED_SESSION());
    uint8_t request[] = {0x02U}; // programming session
    EXPECT_EQ(
        DiagReturnCode::OK,
        fDiagnosticSessionControl.process(fIncomingDiagConnection, request, sizeof(request)));
}

/**
 * \desc
 * process should returns OK, if the requestedSession is an EXTENDED session and if the transition
 * is possible. The current session (default session) is NOT the same as the requested session.
 */
TEST_F(
    DiagnosticSessionControlTest,
    process_should_returns_OK_current_session_is_not_same_as_the_requested_session)
{
    uint8_t const request[] = {0x03U}; // EXTENDED session

    EXPECT_CALL(_asyncMock, schedule(_asyncContext, _, _, 5000, ::async::TimeUnit::MILLISECONDS));
    EXPECT_CALL(fDiagSessionChangedListener, diagSessionChanged(_));

    EXPECT_EQ(
        DiagReturnCode::OK,
        fDiagnosticSessionControl.process(fIncomingDiagConnection, request, sizeof(request)));
}

/**
 * \desc
 * process should returns OK, if the current session is a PROGRAMMING session and the transition to
 * a DEFAULT session is possible.
 */
TEST_F(
    DiagnosticSessionControlTest,
    process_should_returns_OK_if_requestedSession_is_default_session_and_current_session_is_programming)
{
    uint8_t const request[] = {0x01U}; // default session

    fDiagnosticSessionControl.setCurrentSession(DiagSession::PROGRAMMING_SESSION());

    EXPECT_CALL(fDiagSessionChangedListener, diagSessionChanged(_));

    EXPECT_EQ(
        DiagReturnCode::OK,
        fDiagnosticSessionControl.process(fIncomingDiagConnection, request, sizeof(request)));
}

/**
 * \desc
 * process should returns OK, if the requestedSession and the current session are a PROGRAMMING
 * session and if the transition is possible.
 */
TEST_F(
    DiagnosticSessionControlTest,
    process_should_returns_OK_if_the_sessions_are_PROGRAMMING_sessions_and_if_transition_is_possible)
{
    uint8_t const request[] = {0x02U}; // PROGRAMMING session

    fDiagnosticSessionControl.setCurrentSession(DiagSession::PROGRAMMING_SESSION());

    EXPECT_CALL(fDiagSessionChangedListener, diagSessionChanged(_));

    EXPECT_EQ(
        DiagReturnCode::OK,
        fDiagnosticSessionControl.process(fIncomingDiagConnection, request, sizeof(request)));
}

/**
 * \desc
 * process should returns OK, if the requestedSession and the current session are a PROGRAMMING
 * session and if the transition is possible. If a DiagDispatcher exists, process should also
 * disable it.
 */
TEST_F(
    DiagnosticSessionControlTest,
    process_should_disable_dispatcher_if_it_exists_when_transition_into_PROGRAMMING_session_is_possible)
{
    uint8_t const request[] = {0x02U}; // PROGRAMMING session

    fDiagnosticSessionControl.setCurrentSession(DiagSession::PROGRAMMING_SESSION());

    StrictMock<DiagSessionManagerMock> diagSessionManager;
    fDiagnosticSessionControl.setDefaultDiagSessionManager(diagSessionManager);

    DiagJobRoot fDiagJobRoot;

    DiagnosisConfiguration<1, 2, 10> udsConfiguration(
        0x10U,
        0xDFU,
        0u,
        transport::TransportConfiguration::DIAG_PAYLOAD_SIZE,
        true,
        false,
        true,
        static_cast<::async::ContextType>(1U));

    DiagDispatcher2 dispatcher(
        udsConfiguration,
        static_cast<IDiagSessionManager&>(diagSessionManager),
        fDiagJobRoot,
        _asyncContext);

    fDiagnosticSessionControl.setDiagDispatcher(&dispatcher);

    EXPECT_CALL(fDiagSessionChangedListener, diagSessionChanged(_));

    EXPECT_EQ(
        DiagReturnCode::OK,
        fDiagnosticSessionControl.process(fIncomingDiagConnection, request, sizeof(request)));
}

/**
 * \desc
 * process should returns ISO_RESPONSE_TOO_LONG, if maximum length of response is too short.
 */
TEST_F(
    DiagnosticSessionControlTest,
    process_should_returns_ISO_RESPONSE_TOO_LONG_if_maximum_length_of_response_is_too_short)
{
    uint8_t const request[] = {0x01U}; // DEFAULT session

    transport::TransportMessage responseMessage;
    ::etl::array<uint8_t, 4> requestBuffer{};
    responseMessage.init(requestBuffer.data(), requestBuffer.size());
    fIncomingDiagConnection.fpRequestMessage = &responseMessage;

    EXPECT_EQ(
        DiagReturnCode::ISO_RESPONSE_TOO_LONG,
        fDiagnosticSessionControl.process(fIncomingDiagConnection, request, sizeof(request)));
}

/**
 * \desc
 * responseSent should calls startSessionTimeout, if the response is not pending. If
 * fTesterPresentReceived is false, diagSessionResponseSent should be send.
 *
 */
TEST_F(
    DiagnosticSessionControlTest,
    responseSent_should_calls_startSessionTimeout_and_diagSessionResponseSent)
{
    uint8_t const response[] = {0x50U, 0x01U};

    EXPECT_CALL(_asyncMock, schedule(_asyncContext, _, _, 5000, ::async::TimeUnit::MILLISECONDS))
        .Times(2);
    EXPECT_CALL(fDiagSessionChangedListener, diagSessionResponseSent(DiagReturnCode::OK))
        .Times(AtLeast(1));

    // DiagSession::PROGRAMMING_SESSION()
    fDiagnosticSessionControl.setCurrentSession(DiagSession::PROGRAMMING_SESSION());
    fDiagnosticSessionControl.responseSent(
        fIncomingDiagConnection, DiagReturnCode::OK, response, sizeof(response));

    // DiagSession::APPLICATION_EXTENDED_SESSION()
    fDiagnosticSessionControl.setCurrentSession(DiagSession::APPLICATION_EXTENDED_SESSION());
    fDiagnosticSessionControl.responseSent(
        fIncomingDiagConnection, DiagReturnCode::OK, response, sizeof(response));
}

/**
 * \desc
 * responseSent should calls startSessionTimeout, if the response is not pending. If
 * fTesterPresentReceived is true nothing more should do.
 *
 */
TEST_F(DiagnosticSessionControlTest, responseSent_calls_just_startSessionTimeout)
{
    uint8_t const response[] = {0x50U, 0x01U};

    fDiagnosticSessionControl.setCurrentSession(DiagSession::PROGRAMMING_SESSION());
    fDiagnosticSessionControl.setTesterPresentReceived(true);

    EXPECT_CALL(_asyncMock, schedule(_asyncContext, _, _, 5000, ::async::TimeUnit::MILLISECONDS));

    fDiagnosticSessionControl.responseSent(
        fIncomingDiagConnection, DiagReturnCode::OK, response, sizeof(response));
}

/**
 * \desc
 * responseSent should calls startSessionTimeout but no timeout should be set, because the current
 * session is a default session.
 *
 */
TEST_F(
    DiagnosticSessionControlTest,
    responseSent_should_calls_startSessionTimeout_but_no_timeout_should_be_set)
{
    uint8_t const response[] = {0x50U, 0x01U};

    EXPECT_CALL(fDiagSessionChangedListener, diagSessionResponseSent(DiagReturnCode::OK));

    fDiagnosticSessionControl.responseSent(
        fIncomingDiagConnection, DiagReturnCode::OK, response, sizeof(response));
}

/**
 * \desc
 * responseSent should not calls startSessionTimeout, because the result parameter was set to
 * ISO_RESPONSE_PENDING.
 *
 */
TEST_F(
    DiagnosticSessionControlTest,
    responseSent_should_not_calls_startSessionTimeout_because_result_is_ISO_RESPONSE_PENDING)
{
    uint8_t const response[] = {0x50U, 0x01U};

    EXPECT_CALL(
        fDiagSessionChangedListener, diagSessionResponseSent(DiagReturnCode::ISO_RESPONSE_PENDING));

    fDiagnosticSessionControl.responseSent(
        fIncomingDiagConnection, DiagReturnCode::ISO_RESPONSE_PENDING, response, sizeof(response));
}

/**
 * \desc
 * stopSessionTimeout should stop the timeout
 *
 */
TEST_F(DiagnosticSessionControlTest, stopSessionTimeout)
{
    fDiagnosticSessionControl.stopSessionTimeout();
}

/**
 * \desc
 * isSessionTimeoutActive should returns if sessionTimeout is active or not.
 *
 */
TEST_F(DiagnosticSessionControlTest, isSessionTimeoutActive)
{
    EXPECT_FALSE(fDiagnosticSessionControl.isSessionTimeoutActive());
}

/**
 * \desc
 * resetToDefaultSession should switch to default session.
 */
TEST_F(DiagnosticSessionControlTest, resetToDefaultSession_should_switch_to_default_session)
{
    fDiagnosticSessionControl.setCurrentSession(DiagSession::PROGRAMMING_SESSION());

    EXPECT_CALL(fDiagSessionChangedListener, diagSessionChanged(_));

    fDiagnosticSessionControl.resetToDefaultSession();
}

/**
 * \desc
 * acceptedJob should return the DiagReturnCode OK.
 */
TEST_F(DiagnosticSessionControlTest, acceptedJob)
{
    ReadDataByIdentifier readDataByIdentifier;
    uint8_t const request[] = {0x22U, 0x01U, 0x00U};

    EXPECT_EQ(
        DiagReturnCode::OK,
        fDiagnosticSessionControl.acceptedJob(
            fIncomingDiagConnection, readDataByIdentifier, request, sizeof(request)));
}

/**
 * \desc
 * expired should check if the session timeout is allowed for the current session. Since the current
 * session is an APPLICATION_DEFAULT_SESSION(), switching session shouldn't be allowed.
 */
TEST_F(DiagnosticSessionControlTest, expired_session_timeout_for_current_session_is_not_allowed)
{
    fDiagnosticSessionControl.execute();
}

/**
 * \desc
 * expired should check if the session timeout is allowed for the current session. Since the current
 * session is an APPLICATION_EXTENDED_SESSION(), switching to APPLICATION_DEFAULT_SESSION() should
 * be allowed.
 */
TEST_F(DiagnosticSessionControlTest, expired_session_timeout_for_current_session_allowed)
{
    EXPECT_CALL(fDiagSessionChangedListener, diagSessionChanged(_)).Times(AtLeast(1));

    // DiagSession::APPLICATION_EXTENDED_SESSION()
    fDiagnosticSessionControl.setCurrentSession(DiagSession::APPLICATION_EXTENDED_SESSION());
    fDiagnosticSessionControl.execute();
}

/**
 * \desc
 * sessionRead switch to default session and initialize VoidDelegate.
 */
TEST_F(DiagnosticSessionControlTest, sessionRead)
{
    uint8_t const session = 0x41U; // CODING session

    EXPECT_CALL(fDiagSessionChangedListener, diagSessionChanged(_));

    ReadDataByIdentifier readDataByIdentifier;
    ECUReset ecuReset;
    TesterPresent testerPresent;
    using void_function = ::estd::function_mock<void()>;
    void_function ic;
    InitCompleteCallbackType initComplete
        = ::etl::delegate<void()>::create<void_function, &void_function::callee>(ic);
    EXPECT_CALL(ic, callee());
    EXPECT_CALL(fSessionPersistence, readSession(Ref(fDiagnosticSessionControl)));

    fDiagnosticSessionControl.init(
        readDataByIdentifier, ecuReset, fDiagnosticSessionControl, testerPresent, initComplete);

    fDiagnosticSessionControl.sessionRead(session);
}

/**
 * \desc
 * If writing session to eeprom failed, sessionWritten logs an error message.
 */
TEST_F(DiagnosticSessionControlTest, sessionWritten_writing_session_to_eeprom_failed)
{
    fDiagnosticSessionControl.sessionWritten(false);
}

/**
 * \desc
 * sessionWritten should calls requestShutdown, if no session configuration was set.
 */
TEST_F(
    DiagnosticSessionControlTest,
    sessionWritten_should_calls_requestShutdown_because_no_session_configuration_was_set)
{
    EXPECT_CALL(
        fUdsLifecycleConnector,
        requestShutdown(IUdsLifecycleConnector::HARD_RESET, fDiagnosticSessionControl.RESET_TIME))
        .WillOnce(Return(true));
    fDiagnosticSessionControl.sessionWritten(true);
}

} // anonymous namespace
