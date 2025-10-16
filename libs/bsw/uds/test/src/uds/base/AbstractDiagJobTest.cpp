// Copyright 2024 Accenture.

#include "transport/TransportMessageProvidingListenerMock.h"
#include "uds/DiagDispatcher.h"
#include "uds/DiagReturnCode.h"
#include "uds/authentication/DefaultDiagAuthenticator.h"
#include "uds/authentication/DiagAuthenticatorMock.h"
#include "uds/base/DiagJobMock.h"
#include "uds/base/DiagJobRoot.h"
#include "uds/session/ApplicationExtendedSession.h"
#include "uds/session/DiagSessionManagerMock.h"

#include <async/Async.h>
#include <etl/array.h>

#include <gtest/gtest.h>

#include <cstdint>
#include <memory>

namespace
{
using namespace ::uds;
using namespace ::transport;
using namespace ::testing;

class TestableDiagJob : public AbstractDiagJob
{
public:
    explicit TestableDiagJob(AbstractDiagJob& job) : AbstractDiagJob(&job) {}

    TestableDiagJob(
        uint8_t const implementedRequest[],
        uint8_t const requestLength,
        uint8_t const prefixLength,
        DiagSessionMask const sessionMask = DiagSession::ALL_SESSIONS())
    : AbstractDiagJob(implementedRequest, requestLength, prefixLength, sessionMask)
    {}

    TestableDiagJob(
        uint8_t const implementedRequest[],
        uint8_t const requestLength,
        uint8_t const prefixLength,
        uint8_t const requestPayloadLength,
        uint8_t const responseLength,
        DiagSessionMask const sessionMask = DiagSession::ALL_SESSIONS())
    : AbstractDiagJob(
        implementedRequest,
        requestLength,
        prefixLength,
        requestPayloadLength,
        responseLength,
        sessionMask)
    {}

    DiagReturnCode::Type verify(uint8_t const[], uint16_t requestLength) override
    {
        if (requestLength > 0)
        {
            return DiagReturnCode::OK;
        }
        return DiagReturnCode::ISO_INVALID_FORMAT;
    }

    IDiagAuthenticator const& getDiagAuthenticator() const override { return fDiagAuthenticator; }

    StrictMock<DiagAuthenticatorMock> fDiagAuthenticator;

    using AbstractDiagJob::compare;
    using AbstractDiagJob::enableSuppressPositiveResponse;
    using AbstractDiagJob::getAndResetConnection;
    using AbstractDiagJob::getDefaultDiagReturnCode;
    using AbstractDiagJob::getDiagSessionManager;
    using AbstractDiagJob::isFamily;
    using AbstractDiagJob::setEnableSuppressPositiveResponse;
    using AbstractDiagJob::setNextJob;
};

struct AbstractDiagJobTest : public Test
{
    AbstractDiagJobTest()
    {
        fResponseMessage.init(fRequestBuffer.data(), fRequestBuffer.size());

        fpDiagAuthenticator.reset(new DefaultDiagAuthenticator());
        fpDiagSessionManager.reset(new DiagSessionManagerMock());
        AbstractDiagJob::setDefaultDiagSessionManager(*fpDiagSessionManager);

        fIncomingConnection.fSourceId        = 0x10U;
        fIncomingConnection.fpRequestMessage = &fResponseMessage;
    }

    std::unique_ptr<IDiagSessionManager> fpDiagSessionManager;
    std::unique_ptr<IDiagAuthenticator> fpDiagAuthenticator;
    DiagJobRoot fpDiagJobRoot;
    DiagJobRoot fJobRoot;
    TestableDiagJob fTestableDiagJob{fJobRoot};
    IncomingDiagConnection fIncomingConnection{::async::CONTEXT_INVALID};
    StrictMock<DiagSessionManagerMock> fSessionManager;
    transport::TransportMessage fResponseMessage;
    ::etl::array<uint8_t, 6U> fRequestBuffer{};

    static DiagSession::DiagSessionMask const& EMPTY_SESSION_MASK()
    {
        return DiagSession::DiagSessionMask::getInstance();
    }

    static uint8_t const LENGTH_NULL = 0U;
    static uint8_t const IMPLEMENTED_REQUEST[];
    static uint8_t const REQUEST[];
};

uint8_t const AbstractDiagJobTest::REQUEST[]             = {0x22U, 0x01U, 0x00U};
uint8_t const AbstractDiagJobTest::IMPLEMENTED_REQUEST[] = {0x22U};

/**
 * @test
 * Add a single job to the root.
 * @see AbstractDiagJob::addAbstractDiagJob()
 */
TEST_F(AbstractDiagJobTest, addOneJob)
{
    DIAG_JOB(myJob1, {0x22U}, 0U);
    //  DiagJobMock job("myJob1", (uint8_t[]){0x22}, 1, 0);
    // add job to root
    ASSERT_EQ(AbstractDiagJob::JOB_ADDED, fpDiagJobRoot.addAbstractDiagJob(myJob1));
}

/**
 * @test
 * Adding identical jobs must fail. Identical jobs are the same object or
 * jobs with the same implemented request.
 * @see AbstractDiagJob::addAbstractDiagJob()
 */
TEST_F(AbstractDiagJobTest, addIdenticalJobs)
{
    DIAG_JOB(myJob1, {0x22U}, 0U);
    // add job to root
    ASSERT_EQ(AbstractDiagJob::JOB_ADDED, fpDiagJobRoot.addAbstractDiagJob(myJob1));
    // adding the job again must fail
    ASSERT_EQ(AbstractDiagJob::JOB_NOT_ADDED, fpDiagJobRoot.addAbstractDiagJob(myJob1));

    // set up another job with same implemented request
    DIAG_JOB(myJob2, {0x22U}, 0U);
    // adding the second job must fail
    ASSERT_EQ(AbstractDiagJob::JOB_NOT_ADDED, fpDiagJobRoot.addAbstractDiagJob(myJob2));
}

TEST_F(AbstractDiagJobTest, addDifferentJobs)
{
    DIAG_JOB(myJob1, {0x22U}, 0U);
    DIAG_JOB(myJob2, {0x2EU}, 0U);
    ASSERT_EQ(AbstractDiagJob::JOB_ADDED, fpDiagJobRoot.addAbstractDiagJob(myJob1));
    ASSERT_EQ(AbstractDiagJob::JOB_ADDED, fpDiagJobRoot.addAbstractDiagJob(myJob2));
}

TEST_F(AbstractDiagJobTest, add_should_return_JOB_NOT_ADDED_if_the_job_has_the_same_address)
{
    EXPECT_EQ(AbstractDiagJob::JOB_NOT_ADDED, fpDiagJobRoot.addAbstractDiagJob(fpDiagJobRoot));
}

TEST_F(AbstractDiagJobTest, add_returns_JOB_NOT_ADDED_if_adding_a_diagJob_with_same_content)
{
    TestableDiagJob diagJob1(IMPLEMENTED_REQUEST, sizeof(IMPLEMENTED_REQUEST), 0U);
    TestableDiagJob diagJob2(IMPLEMENTED_REQUEST, sizeof(IMPLEMENTED_REQUEST), 0U);

    EXPECT_EQ(AbstractDiagJob::JOB_NOT_ADDED, diagJob1.addAbstractDiagJob(diagJob2));
}

TEST_F(AbstractDiagJobTest, add_returns_JOB_NOT_ADDED_if_job_was_added_earlier)
{
    DIAG_JOB(myJob1, {0x22U}, 0U);
    DIAG_JOB(myJob2, {0x2EU}, 0U);
    DIAG_JOB(myJob3, {0x33U}, 0U);
    DIAG_JOB(myJob4, {0x33U}, 0U);

    EXPECT_EQ(AbstractDiagJob::ErrorCode::JOB_ADDED, fpDiagJobRoot.addAbstractDiagJob(myJob1));
    EXPECT_EQ(AbstractDiagJob::ErrorCode::JOB_ADDED, fpDiagJobRoot.addAbstractDiagJob(myJob2));
    EXPECT_EQ(AbstractDiagJob::ErrorCode::JOB_ADDED, fpDiagJobRoot.addAbstractDiagJob(myJob3));
    EXPECT_EQ(AbstractDiagJob::ErrorCode::JOB_NOT_ADDED, fpDiagJobRoot.addAbstractDiagJob(myJob1));
    EXPECT_EQ(AbstractDiagJob::ErrorCode::JOB_NOT_ADDED, fpDiagJobRoot.addAbstractDiagJob(myJob2));
    EXPECT_EQ(AbstractDiagJob::ErrorCode::JOB_NOT_ADDED, fpDiagJobRoot.addAbstractDiagJob(myJob3));
    EXPECT_EQ(AbstractDiagJob::ErrorCode::JOB_NOT_ADDED, fpDiagJobRoot.addAbstractDiagJob(myJob4));
}

TEST_F(AbstractDiagJobTest, add_returns_JOB_NOT_ADDED_if_job_is_not_family)
{
    uint8_t const request[] = {0x22U, 0x01U, 0xA0U};
    TestableDiagJob diagJob1(request, sizeof(request), 0U);

    TestableDiagJob diagJob2(request, sizeof(request), 1U);

    EXPECT_EQ(AbstractDiagJob::JOB_NOT_ADDED, diagJob1.addAbstractDiagJob(diagJob2));
}

TEST_F(
    AbstractDiagJobTest, add_returns_JOB_NOT_ADDED_if_job_is_family_but_firstChild_does_not_exist)
{
    uint8_t const request[] = {0x22U, 0x01U, 0xA0U};
    TestableDiagJob diagJob1(request, sizeof(request), 1U);

    EXPECT_EQ(AbstractDiagJob::JOB_NOT_ADDED, fpDiagJobRoot.addAbstractDiagJob(diagJob1));
}

TEST_F(
    AbstractDiagJobTest,
    add_returns_JOB_NOT_ADDED_if_job_is_family_but_firstChild_can_not_add_second_job)
{
    uint8_t const request[] = {0x22U, 0x01U, 0xA0U};
    TestableDiagJob diagJob1(request, sizeof(request), 0U);

    TestableDiagJob diagJob2(request, sizeof(request), 2U);

    EXPECT_EQ(AbstractDiagJob::JOB_ADDED, fpDiagJobRoot.addAbstractDiagJob(diagJob1));
    EXPECT_EQ(AbstractDiagJob::JOB_NOT_ADDED, fpDiagJobRoot.addAbstractDiagJob(diagJob2));
}

TEST_F(AbstractDiagJobTest, remove_jobs_in_different_order_should_works_correctly)
{
    DIAG_JOB(myJob1, {0x22U}, 0U);
    DIAG_JOB(myJob2, {0x2EU}, 0U);
    DIAG_JOB(myJob3, {0x31U}, 0U);
    DIAG_JOB(myJob4, {0x3FU}, 0U);
    DIAG_JOB(myJob5, {0x10U}, 0U);
    fpDiagJobRoot.addAbstractDiagJob(myJob1);
    fpDiagJobRoot.addAbstractDiagJob(myJob2);
    fpDiagJobRoot.addAbstractDiagJob(myJob3);
    fpDiagJobRoot.addAbstractDiagJob(myJob4);
    fpDiagJobRoot.addAbstractDiagJob(myJob5);

    // cannot remove us from ourself
    fpDiagJobRoot.removeAbstractDiagJob(fpDiagJobRoot);

    // remove first job
    fpDiagJobRoot.removeAbstractDiagJob(myJob1);

    // remove next job
    fpDiagJobRoot.removeAbstractDiagJob(myJob3);

    // remove last job
    fpDiagJobRoot.removeAbstractDiagJob(myJob5);
}

TEST_F(AbstractDiagJobTest, setEnableSuppressPositiveResponse)
{
    fTestableDiagJob.setEnableSuppressPositiveResponse(fTestableDiagJob);
}

TEST_F(AbstractDiagJobTest, getDefaultDiagReturnCode)
{
    EXPECT_EQ(DiagReturnCode::ISO_GENERAL_REJECT, fTestableDiagJob.getDefaultDiagReturnCode());
}

TEST_F(
    AbstractDiagJobTest,
    constructor_one_calls_an_exception_if_prefixLength_is_greaterEq_requestLength)
{
    ASSERT_THROW(
        { TestableDiagJob diagJob(IMPLEMENTED_REQUEST, sizeof(IMPLEMENTED_REQUEST), 1U); },
        ::etl::exception);
}

TEST_F(
    AbstractDiagJobTest,
    constructor_two_calls_an_exception_if_prefixLength_is_greaterEq_requestLength)
{
    ASSERT_THROW(
        { TestableDiagJob diagJob(IMPLEMENTED_REQUEST, sizeof(IMPLEMENTED_REQUEST), 1U, 0U, 1U); },
        ::etl::exception);
}

TEST_F(
    AbstractDiagJobTest,
    constructor_two_does_not_call_an_exception_if_prefixLength_is_smaller_than_requestLength)
{
    TestableDiagJob diagJob(IMPLEMENTED_REQUEST, sizeof(IMPLEMENTED_REQUEST), 0U, 0U, 1U);
}

TEST_F(AbstractDiagJobTest, constructor_two_does_no_prefixLength_check_if_requestLength_is_zero)
{
    uint8_t const* pNull = 0UL;

    TestableDiagJob diagJob(pNull, sizeof(pNull), 0U, 0U, 1U);
}

TEST_F(
    AbstractDiagJobTest,
    getDiagSessionManager_calls_an_exception_if_a_sessionManager_does_not_exist)
{
    IDiagSessionManager* diagSessionManager = nullptr;
    AbstractDiagJob::setDefaultDiagSessionManager(*diagSessionManager);
    ASSERT_THROW(fTestableDiagJob.getDiagSessionManager(), ::etl::exception);
}

TEST_F(
    AbstractDiagJobTest,
    const_getDiagSessionManager_calls_an_exception_if_a_sessionManager_does_not_exist)
{
    TestableDiagJob const diagJob(IMPLEMENTED_REQUEST, sizeof(IMPLEMENTED_REQUEST), 0U, 0U, 1U);

    IDiagSessionManager* diagSessionManager = nullptr;
    AbstractDiagJob::setDefaultDiagSessionManager(*diagSessionManager);
    ASSERT_THROW(diagJob.getDiagSessionManager(), ::etl::exception);
}

TEST_F(
    AbstractDiagJobTest, setNextJob_does_nothing_if_AbstractDiagJob_as_next_job_tried_to_add_itself)
{
    fTestableDiagJob.setNextJob(&fTestableDiagJob);
}

TEST_F(AbstractDiagJobTest, isFamily_returns_false_if_length_is_greater_than_prefixLength)
{
    EXPECT_FALSE(fTestableDiagJob.isFamily(nullptr, 1U));
}

TEST_F(AbstractDiagJobTest, compare_returns_true_if_both_data_arrays_are_nullpointers)
{
    EXPECT_TRUE(fTestableDiagJob.compare(nullptr, nullptr, 1U));
}

TEST_F(AbstractDiagJobTest, compare_returns_false_if_data_arrays_are_different)
{
    uint8_t const data1[] = {0x22U};
    uint8_t const* data2  = nullptr;

    EXPECT_FALSE(fTestableDiagJob.compare(data2, data1, 1U));

    EXPECT_FALSE(fTestableDiagJob.compare(data1, data2, 1U));
}

TEST_F(
    AbstractDiagJobTest,
    execute_returns_ISO_REQUEST_OUT_OF_RANGE_if_session_does_not_match_and_requestLength_leq_zero)
{
    TestableDiagJob diagJobWithEmptyMask(
        IMPLEMENTED_REQUEST, LENGTH_NULL, 0U, EMPTY_SESSION_MASK());

    diagJobWithEmptyMask.setDefaultDiagSessionManager(fSessionManager);

    EXPECT_CALL(fSessionManager, getActiveSession())
        .WillOnce(ReturnRef(DiagSession::APPLICATION_EXTENDED_SESSION()));

    EXPECT_EQ(
        DiagReturnCode::ISO_REQUEST_OUT_OF_RANGE,
        diagJobWithEmptyMask.execute(fIncomingConnection, REQUEST, sizeof(REQUEST)));
}

TEST_F(
    AbstractDiagJobTest,
    execute_returns_ISO_SECURITY_ACCESS_DENIED_if_authentication_fails_and_requestLength_leq_zero)
{
    TestableDiagJob diagJob(IMPLEMENTED_REQUEST, LENGTH_NULL, 0U);

    diagJob.setDefaultDiagSessionManager(fSessionManager);

    EXPECT_CALL(fSessionManager, getActiveSession())
        .WillOnce(ReturnRef(DiagSession::APPLICATION_EXTENDED_SESSION()));

    EXPECT_CALL(diagJob.fDiagAuthenticator, isAuthenticated(0x10U)).WillOnce(Return(false));

    EXPECT_CALL(diagJob.fDiagAuthenticator, getNotAuthenticatedReturnCode())
        .WillOnce(Return(DiagReturnCode::ISO_SECURITY_ACCESS_DENIED));

    EXPECT_EQ(
        DiagReturnCode::ISO_SECURITY_ACCESS_DENIED,
        diagJob.execute(fIncomingConnection, REQUEST, sizeof(REQUEST)));
}

TEST_F(
    AbstractDiagJobTest,
    execute_returns_ISO_SECURITY_ACCESS_DENIED_if_authentication_fails_and_requestLength_gr_zero)
{
    TestableDiagJob diagJobWithValidRequestSize(
        IMPLEMENTED_REQUEST, sizeof(IMPLEMENTED_REQUEST), 0U);

    diagJobWithValidRequestSize.setDefaultDiagSessionManager(fSessionManager);

    EXPECT_CALL(fSessionManager, getActiveSession())
        .WillOnce(ReturnRef(DiagSession::APPLICATION_EXTENDED_SESSION()));

    EXPECT_CALL(diagJobWithValidRequestSize.fDiagAuthenticator, isAuthenticated(0x10U))
        .WillOnce(Return(false));

    EXPECT_CALL(diagJobWithValidRequestSize.fDiagAuthenticator, getNotAuthenticatedReturnCode())
        .WillOnce(Return(DiagReturnCode::ISO_SECURITY_ACCESS_DENIED));

    EXPECT_CALL(
        fSessionManager,
        acceptedJob(
            Ref(fIncomingConnection), Ref(diagJobWithValidRequestSize), REQUEST, sizeof(REQUEST)))
        .WillOnce(Return(DiagReturnCode::OK));

    EXPECT_EQ(
        DiagReturnCode::ISO_SECURITY_ACCESS_DENIED,
        diagJobWithValidRequestSize.execute(fIncomingConnection, REQUEST, sizeof(REQUEST)));
}

TEST_F(
    AbstractDiagJobTest,
    execute_returns_ISO_INVALID_FORMAT_if_requestPayloadLength_neq_fRequestPayloadLength_but_gr_0)
{
    TestableDiagJob extendedDiagJob(
        IMPLEMENTED_REQUEST,
        sizeof(IMPLEMENTED_REQUEST), // fRequestLength
        0U,                          // fPrefixLength
        0U,                          // fRequestPayloadLength
        1U);                         // fResponseLength

    extendedDiagJob.setDefaultDiagSessionManager(fSessionManager);

    EXPECT_CALL(fSessionManager, getActiveSession())
        .WillOnce(ReturnRef(DiagSession::APPLICATION_EXTENDED_SESSION()));

    EXPECT_CALL(extendedDiagJob.fDiagAuthenticator, isAuthenticated(0x10U)).WillOnce(Return(true));

    EXPECT_CALL(
        fSessionManager,
        acceptedJob(Ref(fIncomingConnection), Ref(extendedDiagJob), REQUEST, sizeof(REQUEST)))
        .WillOnce(Return(DiagReturnCode::OK));

    EXPECT_EQ(
        DiagReturnCode::ISO_INVALID_FORMAT,
        extendedDiagJob.execute(fIncomingConnection, REQUEST, sizeof(REQUEST)));
}

TEST_F(
    AbstractDiagJobTest,
    execute_returns_ISO_INVALID_FORMAT_and_does_not_call_acceptedJob_if_requestLength_is_0)
{
    TestableDiagJob jobWithRequestLengthTwo(IMPLEMENTED_REQUEST, 0U, 1U, 2U, 1U);

    jobWithRequestLengthTwo.setDefaultDiagSessionManager(fSessionManager);

    EXPECT_CALL(fSessionManager, getActiveSession())
        .WillOnce(ReturnRef(DiagSession::APPLICATION_EXTENDED_SESSION()));

    EXPECT_CALL(jobWithRequestLengthTwo.fDiagAuthenticator, isAuthenticated(0x10U))
        .WillOnce(Return(true));

    EXPECT_EQ(
        DiagReturnCode::ISO_INVALID_FORMAT,
        jobWithRequestLengthTwo.execute(fIncomingConnection, REQUEST, sizeof(REQUEST)));
}

TEST_F(
    AbstractDiagJobTest,
    execute_returns_ISO_RESPONSE_TOO_LONG_if_responseLength_is_smaller_then_maxResponseLength)
{
    uint8_t const request[] = {0x22U, 0x0BU};
    TestableDiagJob diagJob(request, sizeof(request), 1U, 2U, 7U);

    diagJob.setDefaultDiagSessionManager(fSessionManager);

    EXPECT_CALL(fSessionManager, getActiveSession())
        .WillOnce(ReturnRef(DiagSession::APPLICATION_EXTENDED_SESSION()));

    EXPECT_CALL(diagJob.fDiagAuthenticator, isAuthenticated(0x10U)).WillOnce(Return(true));

    EXPECT_CALL(
        fSessionManager,
        acceptedJob(Ref(fIncomingConnection), Ref(diagJob), REQUEST, sizeof(REQUEST)))
        .WillOnce(Return(DiagReturnCode::OK));

    EXPECT_EQ(
        DiagReturnCode::ISO_RESPONSE_TOO_LONG,
        diagJob.execute(fIncomingConnection, REQUEST, sizeof(REQUEST)));
}

TEST_F(
    AbstractDiagJobTest,
    execute_returns_ISO_RESPONSE_TOO_LONG_if_responseLength_is_to_small_and_if_fRequestLength_is_gr_0)
{
    uint8_t const request[] = {0x22U, 0x0BU};
    TestableDiagJob diagJob(request, sizeof(request), 1U, 2U, 7U);

    diagJob.setDefaultDiagSessionManager(fSessionManager);

    EXPECT_CALL(fSessionManager, getActiveSession())
        .WillOnce(ReturnRef(DiagSession::APPLICATION_EXTENDED_SESSION()));

    EXPECT_CALL(diagJob.fDiagAuthenticator, isAuthenticated(0x10U)).WillOnce(Return(true));

    EXPECT_CALL(
        fSessionManager,
        acceptedJob(Ref(fIncomingConnection), Ref(diagJob), REQUEST, sizeof(REQUEST)))
        .WillOnce(Return(DiagReturnCode::OK));

    EXPECT_EQ(
        DiagReturnCode::ISO_RESPONSE_TOO_LONG,
        diagJob.execute(fIncomingConnection, REQUEST, sizeof(REQUEST)));
}

TEST_F(
    AbstractDiagJobTest,
    execute_returns_ISO_RESPONSE_TOO_LONG_if_responseLength_is_to_small_and_if_fRequestLength_is_0)
{
    TestableDiagJob diagJob(IMPLEMENTED_REQUEST, LENGTH_NULL, 0U, 3U, 7U);

    diagJob.setDefaultDiagSessionManager(fSessionManager);

    EXPECT_CALL(fSessionManager, getActiveSession())
        .WillOnce(ReturnRef(DiagSession::APPLICATION_EXTENDED_SESSION()));

    EXPECT_CALL(diagJob.fDiagAuthenticator, isAuthenticated(0x10U)).WillOnce(Return(true));

    EXPECT_EQ(
        DiagReturnCode::ISO_RESPONSE_TOO_LONG,
        diagJob.execute(fIncomingConnection, REQUEST, sizeof(REQUEST)));
}

TEST_F(
    AbstractDiagJobTest,
    execute_returns_ISO_GENERAL_REJECT_if_everything_is_working_but_process_fails)
{
    uint8_t const implementedRequest[] = {0x22U, 0x0BU};
    TestableDiagJob diagJob(implementedRequest, sizeof(implementedRequest), 1U, 2U, 1U);

    diagJob.setDefaultDiagSessionManager(fSessionManager);

    EXPECT_CALL(fSessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_EXTENDED_SESSION()));

    EXPECT_CALL(diagJob.fDiagAuthenticator, isAuthenticated(0x10U)).WillRepeatedly(Return(true));

    EXPECT_CALL(fSessionManager, acceptedJob(Ref(fIncomingConnection), Ref(diagJob), _, _))
        .WillRepeatedly(Return(DiagReturnCode::OK));

    // with suppressPositiveResponse = true and request[0] is SUPPRESS_POSITIVE_RESPONSE_MASK
    diagJob.enableSuppressPositiveResponse(true);
    uint8_t const request1[] = {0x22U, 0x80U, 0x01U};
    EXPECT_EQ(
        DiagReturnCode::ISO_GENERAL_REJECT,
        diagJob.execute(fIncomingConnection, request1, sizeof(request1)));

    // with suppressPositiveResponse = true and request[0] is not SUPPRESS_POSITIVE_RESPONSE_MASK
    uint8_t const request2[] = {0x22U, 0x00U, 0x01U};
    EXPECT_EQ(
        DiagReturnCode::ISO_GENERAL_REJECT,
        diagJob.execute(fIncomingConnection, request2, sizeof(request2)));
}

TEST_F(
    AbstractDiagJobTest,
    if_verify_returns_not_OK_and_not_NOT_RESPONSIBLE_acceptedJob_will_be_call_and_execute_returns_status)
{
    TestableDiagJob diagJob(IMPLEMENTED_REQUEST, sizeof(IMPLEMENTED_REQUEST), 0U);

    diagJob.setDefaultDiagSessionManager(fSessionManager);

    EXPECT_CALL(fSessionManager, acceptedJob(Ref(fIncomingConnection), Ref(diagJob), REQUEST, 0))
        .WillOnce(Return(DiagReturnCode::OK));

    EXPECT_EQ(DiagReturnCode::ISO_INVALID_FORMAT, diagJob.execute(fIncomingConnection, REQUEST, 0));
}

TEST_F(AbstractDiagJobTest, comparison_operator)
{
    TestableDiagJob job1(REQUEST, sizeof(REQUEST), 0U);

    TestableDiagJob job2(REQUEST, sizeof(REQUEST), 1U);

    TestableDiagJob job3(REQUEST, sizeof(REQUEST), 0U);

    EXPECT_FALSE(job1 == job2); // prefixLength is different
    EXPECT_TRUE(job1 == job3);  // prefixLength is identical
}

TEST_F(AbstractDiagJobTest, test_getAndResetConnection)
{
    IncomingDiagConnection* connection = nullptr;
    EXPECT_EQ(nullptr, TestableDiagJob::getAndResetConnection(connection));

    connection = &fIncomingConnection;
    EXPECT_EQ(&fIncomingConnection, TestableDiagJob::getAndResetConnection(connection));
    EXPECT_EQ(nullptr, TestableDiagJob::getAndResetConnection(connection));
}

} // anonymous namespace
