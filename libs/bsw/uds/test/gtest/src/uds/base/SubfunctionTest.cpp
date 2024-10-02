// Copyright 2024 Accenture.

#include "uds/base/Subfunction.h"

#include "uds/session/ApplicationDefaultSession.h"
#include "uds/session/DiagSessionManagerMock.h"

#include <gtest/gtest.h>

namespace
{
using namespace ::uds;
using namespace ::testing;

class TestableSubfunction : public Subfunction
{
public:
    TestableSubfunction(
        uint8_t const implementedRequest[], DiagSession::DiagSessionMask sessionMask)
    : Subfunction(implementedRequest, sessionMask)
    {}

    TestableSubfunction(
        uint8_t const implementedRequest[],
        uint8_t const requestPayloadLength,
        uint8_t const responseLength,
        DiagSession::DiagSessionMask sessionMask)
    : Subfunction(implementedRequest, requestPayloadLength, responseLength, sessionMask)
    {}

    DiagReturnCode::Type verify(uint8_t const request[], uint16_t requestLength) override
    {
        return Subfunction::verify(request, requestLength);
    }

    uint8_t const* getImplementedRequest() const { return Subfunction::getImplementedRequest(); }

    uint8_t getRequestLength() const { return Subfunction::getRequestLength(); }
};

struct SubfunctionTest : ::testing::Test
{
    StrictMock<DiagSessionManagerMock> fDiagSessionManager;
    ApplicationDefaultSession fApplicationSession;
};

/**
 * \desc
 * Expect the values which were set in the first constructor.
 */
TEST_F(SubfunctionTest, call_first_constructor_and_expect_that_values_are_set_right)
{
    uint8_t const request[] = {0x22, 0x00};
    TestableSubfunction subfunction(request, DiagSession::ALL_SESSIONS());

    EXPECT_EQ(0x2200U, subfunction.getRequestId());
    ASSERT_EQ(request, subfunction.getImplementedRequest());
    ASSERT_EQ(2U, subfunction.getRequestLength());
}

/**
 * \desc
 * Expect the values which were set in the second constructor.
 */
TEST_F(SubfunctionTest, call_second_constructor_and_expect_that_values_are_set_right)
{
    uint8_t const request[] = {0x31, 0x00};
    TestableSubfunction subfunction(request, 1, 2, DiagSession::ALL_SESSIONS());

    EXPECT_EQ(0x3100U, subfunction.getRequestId());
    ASSERT_EQ(request, subfunction.getImplementedRequest());
    ASSERT_EQ(2U, subfunction.getRequestLength());
}

/**
 * \desc
 * If a request is smaller than the MINIMUM_REQUEST_LENGTH verify shall return ISO_INVALID_FORMAT.
 */
TEST_F(
    SubfunctionTest,
    verify_subfunction_shall_return_INVALID_FORMAT_if_the_requestLenght_is_too_small)
{
    uint8_t const request[] = {0x22, 0x00, 0x01};
    TestableSubfunction subfunction(request, DiagSession::ALL_SESSIONS());

    uint8_t const* subfunctionRequest = nullptr;
    EXPECT_EQ(DiagReturnCode::ISO_INVALID_FORMAT, subfunction.verify(subfunctionRequest, 0));
}

/**
 * \desc
 * If the second byte of the implemented request is not identically to the first byte
 * of the subfunction request, verify should return the Return Code: NOT_RESPONSIBLE
 */
TEST_F(
    SubfunctionTest,
    verify_subfunction_shall_return_NOT_RESPONSIBLE_if_the_subfunction_byte_is_not_identical)
{
    uint8_t const request[] = {0x22, 0x00, 0x01};
    TestableSubfunction subfunction(request, DiagSession::ALL_SESSIONS());

    uint8_t const subfunctionRequest[] = {0x02, 0x01};
    EXPECT_EQ(
        DiagReturnCode::NOT_RESPONSIBLE,
        subfunction.verify(subfunctionRequest, sizeof(subfunctionRequest)));
}

/**
 * \desc
 * If the active session is wrong, verify should return
 * ISO_SUBFUNCTION_NOT_SUPPORTED_IN_ACTIVE_SESSION.
 */
TEST_F(
    SubfunctionTest,
    verify_subfunction_shall_return_SUBFUNCTION_NOT_SUPPORTED_IN_ACTIVE_SESSION_if_the_session_is_wrong)
{
    uint8_t const request[] = {0x22, 0x00, 0x01};
    TestableSubfunction subfunction(request, DiagSession::APPLICATION_EXTENDED_SESSION_MASK());

    uint8_t const subfunctionRequest[] = {request[1], request[2]};

    subfunction.setDefaultDiagSessionManager(fDiagSessionManager);
    EXPECT_CALL(fDiagSessionManager, getActiveSession()).WillOnce(ReturnRef(fApplicationSession));

    EXPECT_EQ(
        DiagReturnCode::ISO_SUBFUNCTION_NOT_SUPPORTED_IN_ACTIVE_SESSION,
        subfunction.verify(subfunctionRequest, sizeof(subfunctionRequest)));
}

/**
 * \desc
 * Verify should return OK, if the implemented request and the subfunction request are valid.
 * The active session has also to be allowed.
 */
TEST_F(SubfunctionTest, verify_shall_return_OK_if_all_conditions_are_fulfill)
{
    uint8_t const request[] = {0x22, 0x00, 0x01};
    TestableSubfunction subfunction(request, DiagSession::ALL_SESSIONS());

    uint8_t const subfunctionRequest[] = {request[1], request[2]};

    subfunction.setDefaultDiagSessionManager(fDiagSessionManager);
    EXPECT_CALL(fDiagSessionManager, getActiveSession()).WillOnce(ReturnRef(fApplicationSession));

    EXPECT_EQ(
        DiagReturnCode::OK, subfunction.verify(subfunctionRequest, sizeof(subfunctionRequest)));
}

} // anonymous namespace
