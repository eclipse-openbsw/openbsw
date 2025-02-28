// Copyright 2024 Accenture.

#include "uds/jobs/RoutineControlJob.h"

#include "estd/array.h"
#include "transport/TransportMessage.h"
#include "uds/connection/IncomingDiagConnection.h"
#include "uds/session/ApplicationDefaultSession.h"
#include "uds/session/DiagSessionManagerMock.h"

#include <gtest/gtest.h>

namespace
{
using namespace ::uds;
using namespace ::testing;

class TestableRoutineControlJob : public RoutineControlJob
{
public:
    TestableRoutineControlJob(bool null = false)
    : RoutineControlJob(
        DEFAULT_START_ROUTINE_REQUEST,
        sizeof(DEFAULT_START_ROUTINE_REQUEST),
        null ? nullptr : &fStopRoutine,
        null ? nullptr : &fRequestRoutineResults)
    , fStopRoutine(DEFAULT_STOP_ROUTINE_REQUEST, *this)
    , fRequestRoutineResults(DEFAULT_REQUEST_ROUTINE_RESULTS_REQUEST, *this)
    {}

    TestableRoutineControlJob(
        uint8_t const implementedRequest[],
        uint8_t const implementedRequestLength,
        uint8_t const requestPayloadLength,
        uint8_t const responseLength)
    : RoutineControlJob(
        implementedRequest,
        implementedRequestLength,
        requestPayloadLength,
        responseLength,
        &fStopRoutine,
        &fRequestRoutineResults)
    , fStopRoutine(DEFAULT_STOP_ROUTINE_REQUEST, *this)
    , fRequestRoutineResults(DEFAULT_REQUEST_ROUTINE_RESULTS_REQUEST, *this)
    {}

    DiagReturnCode::Type start(
        IncomingDiagConnection& connection,
        uint8_t const request[],
        uint16_t requestLength) override
    {
        return DiagReturnCode::OK;
    }

    uint8_t const* getImplementedRequest() const
    {
        return RoutineControlJob::getImplementedRequest();
    }

    uint8_t getRequestLength() const { return RoutineControlJob::getRequestLength(); }

    DiagReturnCode::Type verify(uint8_t const request[], uint16_t requestLength) override
    {
        return RoutineControlJob::verify(request, requestLength);
    }

    bool disableSequenceCheck()
    {
        RoutineControlJob::disableSequenceCheck();
        return RoutineControlJob::fSequenceCheckEnabled;
    }

    bool setStarted() { return RoutineControlJob::fStarted = true; }

    bool isStarted() const { return RoutineControlJob::fStarted; }

    RoutineControlJobNode fStopRoutine;
    RoutineControlJobNode fRequestRoutineResults;

    static uint8_t const ROUTINE_IDENTIFIER[2];
    static uint8_t const DEFAULT_START_ROUTINE_REQUEST[4];

private:
    static uint8_t const DEFAULT_STOP_ROUTINE_REQUEST[4];
    static uint8_t const DEFAULT_REQUEST_ROUTINE_RESULTS_REQUEST[4];
};

uint8_t const TestableRoutineControlJob::ROUTINE_IDENTIFIER[2] = {0xF0U, 0x0AU};

uint8_t const TestableRoutineControlJob::DEFAULT_START_ROUTINE_REQUEST[4] = {
    0x31U, 0x01U, 0xF0U, 0x0AU // erase EEPROM
};
uint8_t const TestableRoutineControlJob::DEFAULT_STOP_ROUTINE_REQUEST[4]
    = {0x31U, 0x02U, 0xF0U, 0x0AU};
uint8_t const TestableRoutineControlJob::DEFAULT_REQUEST_ROUTINE_RESULTS_REQUEST[4]
    = {0x31U, 0x03U, 0xF0U, 0x0AU};

class TestableStop : public TestableRoutineControlJob
{
public:
    TestableStop() : TestableRoutineControlJob() {}

    MOCK_METHOD(
        DiagReturnCode::Type,
        stop,
        (IncomingDiagConnection & connection,
         uint8_t const* const request,
         uint16_t const requestLength),
        (override));
};

struct RoutineControlJobTest : ::testing::Test
{
    RoutineControlJobTest()
    {
        fMessage.init(fRequestBuffer.data(), fRequestBuffer.size());
        fRequestBuffer[0] = 0x31U;       // erase Eeprom
        fRequestBuffer[1] = DUMMY_VALUE; // default value which have to be set in the tests
        fRequestBuffer[2] = TestableRoutineControlJob::ROUTINE_IDENTIFIER[0];
        fRequestBuffer[3] = TestableRoutineControlJob::ROUTINE_IDENTIFIER[1];

        fRoutineControlJobExtended.setDefaultDiagSessionManager(fDiagSessionManager);
        fIncomingDiagConnection.fpRequestMessage = &fMessage;
    }

    transport::TransportMessage fMessage;
    ::estd::array<uint8_t, 6> fRequestBuffer;

    static uint8_t const DUMMY_VALUE = 0xFFU;

    TestableRoutineControlJob fRoutineControlJob;
    TestableRoutineControlJob fRoutineControlJobExtended{
        TestableRoutineControlJob::DEFAULT_START_ROUTINE_REQUEST,
        sizeof(TestableRoutineControlJob::DEFAULT_START_ROUTINE_REQUEST),
        AbstractDiagJob::VARIABLE_REQUEST_LENGTH,
        AbstractDiagJob::VARIABLE_RESPONSE_LENGTH};

    IncomingDiagConnection fIncomingDiagConnection{::async::CONTEXT_INVALID};
    StrictMock<DiagSessionManagerMock> fDiagSessionManager;
    ApplicationDefaultSession fApplicationSession;
    TestableStop StopRoutine;
};

/**
 * \desc
 * Expect the values which were set in the first constructor.
 */
TEST_F(RoutineControlJobTest, call_first_constructor_and_expect_that_values_are_set_right)
{
    EXPECT_EQ(0x3101F00AU, fRoutineControlJob.getRequestId());
    EXPECT_EQ(
        TestableRoutineControlJob::DEFAULT_START_ROUTINE_REQUEST,
        fRoutineControlJob.getImplementedRequest());
    EXPECT_EQ(
        sizeof(TestableRoutineControlJob::DEFAULT_START_ROUTINE_REQUEST),
        fRoutineControlJob.getRequestLength());
}

/**
 * \desc
 * Expect the values which were set in the second constructor.
 */
TEST_F(RoutineControlJobTest, call_second_constructor_and_expect_that_values_are_set_right)
{
    EXPECT_EQ(0x3101F00AU, fRoutineControlJobExtended.getRequestId());
    EXPECT_EQ(
        TestableRoutineControlJob::DEFAULT_START_ROUTINE_REQUEST,
        fRoutineControlJobExtended.getImplementedRequest());
    EXPECT_EQ(
        sizeof(TestableRoutineControlJob::DEFAULT_START_ROUTINE_REQUEST),
        fRoutineControlJobExtended.getRequestLength());
}

/**
 * \desc
 * Verify Routine Identifier. Expect that byte 3 + 4 of the implemented request is
 * identical to the request parameter in verify.
 */
TEST_F(RoutineControlJobTest, verify_routine_identifier_and_expect_DiagReturnCode_OK)
{
    EXPECT_EQ(
        DiagReturnCode::OK,
        fRoutineControlJob.verify(
            TestableRoutineControlJob::ROUTINE_IDENTIFIER,
            sizeof(TestableRoutineControlJob::ROUTINE_IDENTIFIER)));
}

/**
 * \desc
 * Verify Routine Identifier and expect DiagReturnCode NOT_RESPONSIBLE, because the routine
 * identifier of the implemented request is different compare to the request parameter of "verify".
 */
TEST_F(
    RoutineControlJobTest,
    verify_routine_identifier_and_expect_DiagReturnCode_NOT_RESPONSIBLE_because_request_is_different)
{
    uint8_t const request[] = {0xF1U, 0x0AU};

    EXPECT_EQ(DiagReturnCode::NOT_RESPONSIBLE, fRoutineControlJob.verify(request, sizeof(request)));
}

/**
 * \desc
 * When the abstract class "RoutineControlJob" calls the stop function the DiagReturnCode
 * ISO_CONDITIONS_NOT_CORRECT is expected.
 */
TEST_F(RoutineControlJobTest, stop_should_returns_ISO_CONDITIONS_NOT_CORRECT)
{
    EXPECT_EQ(
        DiagReturnCode::ISO_CONDITIONS_NOT_CORRECT,
        fRoutineControlJob.stop(
            fIncomingDiagConnection,
            TestableRoutineControlJob::DEFAULT_START_ROUTINE_REQUEST,
            sizeof(TestableRoutineControlJob::DEFAULT_START_ROUTINE_REQUEST)));
}

/**
 * \desc
 * When the abstract class "RoutineControlJob" calls the requestResults function the DiagReturnCode
 * ISO_CONDITIONS_NOT_CORRECT is expected.
 */
TEST_F(RoutineControlJobTest, requestResults_should_return_ISO_CONDITIONS_NOT_CORRECT)
{
    EXPECT_EQ(
        DiagReturnCode::ISO_CONDITIONS_NOT_CORRECT,
        fRoutineControlJob.stop(
            fIncomingDiagConnection,
            TestableRoutineControlJob::DEFAULT_START_ROUTINE_REQUEST,
            sizeof(TestableRoutineControlJob::DEFAULT_START_ROUTINE_REQUEST)));
}

/**
 * \desc
 * After disabling SequenceCheck, the value of fSequenceCheckEnabled should be false.
 */
TEST_F(RoutineControlJobTest, disableSequenceCheck)
{
    EXPECT_FALSE(fRoutineControlJob.disableSequenceCheck());
}

/**
 * \desc
 * getStartRoutine should return the right address of RoutineControlJob.
 */
TEST_F(RoutineControlJobTest, getStartRoutine)
{
    EXPECT_EQ(&fRoutineControlJob, &fRoutineControlJob.getStartRoutine());
}

/**
 * \desc
 * getStopRoutine should return the same StopRoutine, which was set in the constructor.
 */
TEST_F(RoutineControlJobTest, getStopRoutine)
{
    EXPECT_EQ(&fRoutineControlJob.fStopRoutine, &fRoutineControlJob.getStopRoutine());
}

/**
 * \desc
 * If RoutineControlJobNode fStopRoutine is set to a nullpointer, getStopRoutine should
 * call an exception.
 */
TEST_F(
    RoutineControlJobTest,
    getStopRoutine_should_call_an_exception_if_RoutineControlJobNode_is_a_nullpointer)
{
    TestableRoutineControlJob routineControlJob(true);

    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    ASSERT_THROW(routineControlJob.getStopRoutine(), ::estd::assert_exception);
}

/**
 * \desc
 * getRequestRoutineResults should return the same RequestRoutineResults, which were set in the
 * constructor.
 */
TEST_F(RoutineControlJobTest, getRequestRoutineResults)
{
    EXPECT_EQ(
        &fRoutineControlJob.fRequestRoutineResults, &fRoutineControlJob.getRequestRoutineResults());
}

/**
 * \desc
 * If RoutineControlJobNode fRequestRoutineResults is set to a nullpointer,
 * getRequestRoutineResults should call an exception.
 */
TEST_F(
    RoutineControlJobTest,
    getRequestRoutineResults_should_call_an_exception_if_RoutineControlJobNode_is_a_nullpointer)
{
    TestableRoutineControlJob routineControlJob("NullPointerTest");

    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    ASSERT_THROW(routineControlJob.getRequestRoutineResults(), ::estd::assert_exception);
}

/**
 * \desc
 * If process get an incoming connection, where the identifier was set to 0x01 (start routine),
 * process should return the DiagReturnCode OK.
 */
TEST_F(RoutineControlJobTest, process_should_return_OK_if_identifier_is_start_routine)
{
    fRequestBuffer[1] = 0x01U; // start routine

    EXPECT_CALL(fDiagSessionManager, getActiveSession()).WillOnce(ReturnRef(fApplicationSession));

    EXPECT_CALL(
        fDiagSessionManager,
        acceptedJob(Ref(fIncomingDiagConnection), Ref(fRoutineControlJobExtended), NotNull(), 0))
        .WillOnce(Return(uds::DiagReturnCode::OK));

    EXPECT_EQ(
        DiagReturnCode::OK,
        fRoutineControlJobExtended.execute(
            fIncomingDiagConnection,
            TestableRoutineControlJob::ROUTINE_IDENTIFIER,
            sizeof(TestableRoutineControlJob::ROUTINE_IDENTIFIER)));
}

/**
 * \desc
 * If process get an incoming connection, where the identifier was set to 0x02 (stop routine),
 * process should return the DiagReturnCode ISO_REQUEST_SEQUENCE_ERROR, if fStarted wasn't set to
 * true before or if fSequenceCheckEnabled was set to true.
 */
TEST_F(
    RoutineControlJobTest,
    process_should_return_ISO_REQUEST_SEQUENCE_ERROR_if_identifier_is_stop_routine)
{
    fRequestBuffer[1] = 0x02U; // stop routine

    EXPECT_CALL(fDiagSessionManager, getActiveSession()).WillOnce(ReturnRef(fApplicationSession));

    EXPECT_CALL(
        fDiagSessionManager,
        acceptedJob(Ref(fIncomingDiagConnection), Ref(fRoutineControlJobExtended), NotNull(), 0))
        .WillOnce(Return(uds::DiagReturnCode::OK));

    EXPECT_EQ(
        DiagReturnCode::ISO_REQUEST_SEQUENCE_ERROR,
        fRoutineControlJobExtended.execute(
            fIncomingDiagConnection,
            TestableRoutineControlJob::ROUTINE_IDENTIFIER,
            sizeof(TestableRoutineControlJob::ROUTINE_IDENTIFIER)));
}

/**
 * \desc
 * If process get an incoming connection, where the identifier was set to 0x02 (stop routine),
 * process should return the DiagReturnCode ISO_CONDITIONS_NOT_CORRECT, if fSequenceCheckEnabled
 * was set to false.
 */
TEST_F(
    RoutineControlJobTest,
    process_should_return_ISO_CONDITIONS_NOT_CORRECT_if_identifier_is_stop_routine_sequenceCheck_disabled)
{
    fRequestBuffer[1] = 0x02U; // stop routine

    EXPECT_CALL(fDiagSessionManager, getActiveSession()).WillOnce(ReturnRef(fApplicationSession));

    EXPECT_CALL(
        fDiagSessionManager,
        acceptedJob(Ref(fIncomingDiagConnection), Ref(fRoutineControlJobExtended), NotNull(), 0))
        .WillOnce(Return(uds::DiagReturnCode::OK));

    EXPECT_FALSE(fRoutineControlJobExtended.disableSequenceCheck());

    EXPECT_EQ(
        DiagReturnCode::ISO_CONDITIONS_NOT_CORRECT,
        fRoutineControlJobExtended.execute(
            fIncomingDiagConnection,
            TestableRoutineControlJob::ROUTINE_IDENTIFIER,
            sizeof(TestableRoutineControlJob::ROUTINE_IDENTIFIER)));
}

/**
 * \desc
 * If process get an incoming connection, where the identifier was set to 0x02 (stop routine),
 * process should return the DiagReturnCode ISO_CONDITIONS_NOT_CORRECT, if fStarted
 * was set to true and if calling stop wasn't successful.
 */
TEST_F(
    RoutineControlJobTest,
    process_should_return_ISO_CONDITIONS_NOT_CORRECT_if_identifier_is_stop_routine_and_stop_failed)
{
    fRequestBuffer[1] = 0x02U; // stop routine

    EXPECT_CALL(fDiagSessionManager, getActiveSession()).WillOnce(ReturnRef(fApplicationSession));

    EXPECT_CALL(
        fDiagSessionManager,
        acceptedJob(Ref(fIncomingDiagConnection), Ref(fRoutineControlJobExtended), NotNull(), 0))
        .WillOnce(Return(uds::DiagReturnCode::OK));

    EXPECT_TRUE(fRoutineControlJobExtended.setStarted());

    EXPECT_EQ(
        DiagReturnCode::ISO_CONDITIONS_NOT_CORRECT,
        fRoutineControlJobExtended.execute(
            fIncomingDiagConnection,
            TestableRoutineControlJob::ROUTINE_IDENTIFIER,
            sizeof(TestableRoutineControlJob::ROUTINE_IDENTIFIER)));
}

/**
 * \desc
 * If process get an incoming connection, where the identifier was set to 0x03 (request routine
 * results), process should return the DiagReturnCode ISO_REQUEST_SEQUENCE_ERROR, if
 * fStarted wasn't set to true before or if fSequenceCheckEnabled was set to true.
 */
TEST_F(
    RoutineControlJobTest,
    process_should_return_ISO_REQUEST_SEQUENCE_ERROR_if_identifier_is_request_routine_results)
{
    fRequestBuffer[1] = 0x03U; // request routine results

    EXPECT_CALL(fDiagSessionManager, getActiveSession()).WillOnce(ReturnRef(fApplicationSession));

    EXPECT_CALL(
        fDiagSessionManager,
        acceptedJob(Ref(fIncomingDiagConnection), Ref(fRoutineControlJobExtended), NotNull(), 0))
        .WillOnce(Return(uds::DiagReturnCode::OK));

    EXPECT_EQ(
        DiagReturnCode::ISO_REQUEST_SEQUENCE_ERROR,
        fRoutineControlJobExtended.execute(
            fIncomingDiagConnection,
            TestableRoutineControlJob::ROUTINE_IDENTIFIER,
            sizeof(TestableRoutineControlJob::ROUTINE_IDENTIFIER)));
}

/**
 * \desc
 * If process get an incoming connection, where the identifier was set to 0x03 (request routine
 * results), process should return the result of "requestResults", if
 * fSequenceCheckEnabled was set to false.
 */
TEST_F(RoutineControlJobTest, process_request_routine_results_sequenceCheck_was_disabled)
{
    fRequestBuffer[1] = 0x03U; // request routine results

    EXPECT_CALL(fDiagSessionManager, getActiveSession()).WillOnce(ReturnRef(fApplicationSession));

    EXPECT_CALL(
        fDiagSessionManager,
        acceptedJob(Ref(fIncomingDiagConnection), Ref(fRoutineControlJobExtended), NotNull(), 0))
        .WillOnce(Return(uds::DiagReturnCode::OK));

    EXPECT_FALSE(fRoutineControlJobExtended.disableSequenceCheck());

    EXPECT_EQ(
        DiagReturnCode::ISO_CONDITIONS_NOT_CORRECT,
        fRoutineControlJobExtended.execute(
            fIncomingDiagConnection,
            TestableRoutineControlJob::ROUTINE_IDENTIFIER,
            sizeof(TestableRoutineControlJob::ROUTINE_IDENTIFIER)));
}

/**
 * \desc
 * If process get an incoming connection, where the identifier was set to 0x03 (request routine
 * results), process should return the result of "requestResults", if fStarted
 * was set to true.
 */
TEST_F(RoutineControlJobTest, process_request_routine_results_fStarted_was_set_to_true)
{
    fRequestBuffer[1] = 0x03U; // request routine results

    EXPECT_CALL(fDiagSessionManager, getActiveSession()).WillOnce(ReturnRef(fApplicationSession));

    EXPECT_CALL(
        fDiagSessionManager,
        acceptedJob(Ref(fIncomingDiagConnection), Ref(fRoutineControlJobExtended), NotNull(), 0))
        .WillOnce(Return(uds::DiagReturnCode::OK));

    EXPECT_TRUE(fRoutineControlJobExtended.setStarted());

    EXPECT_EQ(
        DiagReturnCode::ISO_CONDITIONS_NOT_CORRECT,
        fRoutineControlJobExtended.execute(
            fIncomingDiagConnection,
            TestableRoutineControlJob::ROUTINE_IDENTIFIER,
            sizeof(TestableRoutineControlJob::ROUTINE_IDENTIFIER)));
}

/**
 * \desc
 * If process get an incoming connection, where the identifier isn't 0x01, 0x02 or 0x03, process
 * should return the DiagReturnCode ISO_SUBFUNCTION_NOT_SUPPORTED.
 */
TEST_F(RoutineControlJobTest, process_subfunction_not_supported)
{
    EXPECT_CALL(fDiagSessionManager, getActiveSession()).WillOnce(ReturnRef(fApplicationSession));

    EXPECT_CALL(
        fDiagSessionManager,
        acceptedJob(Ref(fIncomingDiagConnection), Ref(fRoutineControlJobExtended), NotNull(), 0))
        .WillOnce(Return(uds::DiagReturnCode::OK));

    EXPECT_EQ(
        DiagReturnCode::ISO_SUBFUNCTION_NOT_SUPPORTED,
        fRoutineControlJobExtended.execute(
            fIncomingDiagConnection,
            TestableRoutineControlJob::ROUTINE_IDENTIFIER,
            sizeof(TestableRoutineControlJob::ROUTINE_IDENTIFIER)));
}

TEST_F(
    RoutineControlJobTest,
    process_should_return_ISO_CONDITIONS_NOT_CORRECT_if_identifier_is_stop_routine)
{
    fRequestBuffer[1] = 0x02U; // stop routine

    EXPECT_CALL(fDiagSessionManager, getActiveSession()).WillOnce(ReturnRef(fApplicationSession));

    EXPECT_CALL(
        fDiagSessionManager,
        acceptedJob(
            Ref(fIncomingDiagConnection),
            Ref(fRoutineControlJobExtended.getStopRoutine()),
            NotNull(),
            0))
        .WillOnce(Return(uds::DiagReturnCode::OK));

    EXPECT_TRUE(fRoutineControlJobExtended.setStarted());

    EXPECT_EQ(
        DiagReturnCode::ISO_CONDITIONS_NOT_CORRECT,
        fRoutineControlJobExtended.getStopRoutine().execute(
            fIncomingDiagConnection,
            TestableRoutineControlJob::ROUTINE_IDENTIFIER,
            sizeof(TestableRoutineControlJob::ROUTINE_IDENTIFIER)));
}

TEST_F(RoutineControlJobTest, process_should_return_OK_if_stop_is_returning_OK)
{
    fRequestBuffer[1] = 0x02U; // stop routine

    EXPECT_CALL(fDiagSessionManager, getActiveSession()).WillOnce(ReturnRef(fApplicationSession));

    EXPECT_CALL(
        fDiagSessionManager,
        acceptedJob(Ref(fIncomingDiagConnection), Ref(StopRoutine), NotNull(), 0))
        .WillOnce(Return(uds::DiagReturnCode::OK));

    EXPECT_TRUE(StopRoutine.setStarted());

    EXPECT_CALL(StopRoutine, stop(_, _, _)).WillOnce(Return(DiagReturnCode::OK));

    EXPECT_EQ(
        DiagReturnCode::OK,
        StopRoutine.execute(
            fIncomingDiagConnection,
            TestableRoutineControlJob::ROUTINE_IDENTIFIER,
            sizeof(TestableRoutineControlJob::ROUTINE_IDENTIFIER)));

    EXPECT_FALSE(StopRoutine.isStarted());
}
} // anonymous namespace
