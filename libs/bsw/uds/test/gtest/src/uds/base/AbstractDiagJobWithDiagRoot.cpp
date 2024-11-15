// Copyright 2024 Accenture.

#include "uds/DiagDispatcher.h"
#include "uds/DiagReturnCode.h"
#include "uds/authentication/DefaultDiagAuthenticator.h"
#include "uds/authentication/DiagAuthenticatorMock.h"
#include "uds/base/DiagJobMock.h"
#include "uds/base/DiagJobRoot.h"
#include "uds/session/ApplicationDefaultSession.h"
#include "uds/session/ApplicationExtendedSession.h"
#include "uds/session/DiagSessionManagerMock.h"

#include <etl/array.h>

#include <gtest/gtest.h>

#include <cstdint>
#include <memory>

namespace
{
using namespace ::uds;
using namespace ::transport;
using namespace ::testing;

class DiagJobRootWithSupLink : public DiagJobRoot
{
public:
    virtual DiagReturnCode::Type verify(uint8_t const request[], uint16_t requestLength)
    {
        if (getSession() == DiagSession::APPLICATION_EXTENDED_SESSION())
        {
            return DiagJobRoot::verify(request, requestLength);
        }
        return DiagReturnCode::ISO_REQUEST_OUT_OF_RANGE;
    }

    virtual DiagReturnCode::Type
    verifySupplierIndication(uint8_t const* const request, uint16_t const requestLength)
    {
        if (requestLength == 3 && request[0] == 0x22 && request[1] == 0xab && request[2] == 0xcd)
        {
            return DiagReturnCode::ISO_REQUEST_SEQUENCE_ERROR;
        }
        return DiagReturnCode::OK;
    }
};

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

struct AbstractDiagJobWithSupLinkTest : public Test
{
    AbstractDiagJobWithSupLinkTest()
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
    DiagJobRootWithSupLink fJobRoot;
    TestableDiagJob fTestableDiagJob{fJobRoot};
    IncomingDiagConnection fIncomingConnection{::async::CONTEXT_INVALID};
    StrictMock<DiagSessionManagerMock> fSessionManager;
    transport::TransportMessage fResponseMessage;
    ::etl::array<uint8_t, 6U> fRequestBuffer{};

    static DiagSession::DiagSessionMask const& EMPTY_SESSION_MASK()
    {
        return DiagSession::DiagSessionMask::getInstance();
    }
};

TEST_F(AbstractDiagJobWithSupLinkTest, execute_returns_NRC_depending_on_job_sid_did)
{
    uint8_t const implementedRequest[] = {0x22U, 0x0BU, 0x0AU};
    TestableDiagJob diagJob(implementedRequest, sizeof(implementedRequest), 0U);

    diagJob.setDefaultDiagSessionManager(fSessionManager);

    EXPECT_CALL(fSessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_EXTENDED_SESSION()));

    EXPECT_CALL(diagJob.fDiagAuthenticator, isAuthenticated(0x10U)).WillRepeatedly(Return(true));

    EXPECT_CALL(fSessionManager, acceptedJob(Ref(fIncomingConnection), Ref(diagJob), _, _))
        .WillRepeatedly(Return(DiagReturnCode::OK));

    diagJob.enableSuppressPositiveResponse(false);
    uint8_t const request1[] = {0x22U, 0xabU, 0xcdU};
    DiagReturnCode::Type rt  = diagJob.execute(fIncomingConnection, request1, sizeof(request1));

    EXPECT_EQ(DiagReturnCode::ISO_REQUEST_SEQUENCE_ERROR, rt);

    uint8_t const request2[] = {0x22U, 0x0aU, 0x0bU};

    rt = diagJob.execute(fIncomingConnection, request2, sizeof(request2));
    EXPECT_EQ(DiagReturnCode::ISO_GENERAL_REJECT, rt);
}

TEST_F(AbstractDiagJobWithSupLinkTest, execute_returns_NRC_diag_root_wrong_session)
{
    uint8_t const implementedRequest[] = {0x22U, 0x0BU, 0x0AU};
    TestableDiagJob diagJob(implementedRequest, sizeof(implementedRequest), 0U);

    diagJob.setDefaultDiagSessionManager(fSessionManager);

    EXPECT_CALL(fSessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));

    EXPECT_CALL(diagJob.fDiagAuthenticator, isAuthenticated(0x10U)).WillRepeatedly(Return(true));

    EXPECT_CALL(fSessionManager, acceptedJob(Ref(fIncomingConnection), Ref(diagJob), _, _))
        .WillRepeatedly(Return(DiagReturnCode::ISO_GENERAL_REJECT));

    uint8_t const request1[] = {0x22U, 0xabU, 0xcdU};
    DiagReturnCode::Type rt  = diagJob.execute(fIncomingConnection, request1, sizeof(request1));

    EXPECT_EQ(DiagReturnCode::ISO_REQUEST_SEQUENCE_ERROR, rt);
}

} // anonymous namespace
