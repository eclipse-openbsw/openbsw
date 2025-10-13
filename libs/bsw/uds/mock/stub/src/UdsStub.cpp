// Copyright 2024 Accenture.

#include "UdsStub.h"

#include "uds/base/AbstractDiagJob.h"
#include "uds/connection/IncomingDiagConnection.h"
#include "uds/connection/OutgoingDiagConnection.h"
#include "uds/connection/PositiveResponse.h"
#include "uds/session/IDiagSessionChangedListener.h"

#include <transport/TransportMessage.h>
#include <util/Mask.h>

/*
 * DSI: mock area to satisfy dependencies into UDS space
 */
namespace uds
{

static size_t positiveResponseLength = 0;

void PositiveResponse::reset() { positiveResponseLength = 0; }

uint8_t* PositiveResponse::getData()
{
    static uint8_t rspbuf[256];
    return rspbuf;
}

size_t PositiveResponse::getLength() const { return positiveResponseLength; }

size_t PositiveResponse::increaseDataLength(size_t length)
{
    uint16_t const newLength = getLength() + length;
    positiveResponseLength   = newLength;
    return newLength;
}

::uds::ErrorCode IncomingDiagConnection::sendPositiveResponse(AbstractDiagJob& sender)
{
    return ::uds::ErrorCode::OK;
}

PositiveResponse& IncomingDiagConnection::releaseRequestGetResponse() { return _positiveResponse; }

void IncomingDiagConnection::transportMessageProcessed(
    transport::TransportMessage&, ProcessingResult status)
{}

void IncomingDiagConnection::triggerNextNestedRequest() {}

uint16_t IncomingDiagConnection::getMaximumResponseLength() const
{
    return 256; // for test in svcDcmAdapter
}

void IncomingDiagConnection::expired(::async::RunnableType const& timeout) {}

void IncomingDiagConnection::asyncTransportMessageProcessed(
    transport::TransportMessage* pTransportMessage, ProcessingResult const status)
{}

void IncomingDiagConnection::asyncSendPositiveResponse(uint16_t length, AbstractDiagJob* pSender) {}

void IncomingDiagConnection::asyncSendNegativeResponse(
    uint8_t responseCode, AbstractDiagJob* pSender)
{}

::uds::ErrorCode
IncomingDiagConnection::sendPositiveResponseInternal(uint16_t const length, AbstractDiagJob& sender)
{
    return ::uds::ErrorCode::OK;
}

void IncomingDiagConnection::open(bool activatePending)
{
    open                        = true;
    _pendingActivated           = activatePending;
    _suppressPositiveResponse   = false;
    _responsePendingSent        = false;
    _responsePendingIsBeingSent = false;
    _isResponseActive           = false;
}

void IncomingDiagConnection::terminate() { open = false; }

/*
 * class DiagSession
 */
DiagSession::DiagSessionMask const& DiagSession::ALL_SESSIONS()
{
    static DiagSessionMask const allSessions;
    return allSessions;
}

ApplicationDefaultSession& DiagSession::APPLICATION_DEFAULT_SESSION()
{
    static ApplicationDefaultSession applicationSession;
    return applicationSession;
}

ApplicationExtendedSession& DiagSession::APPLICATION_EXTENDED_SESSION()
{
    static ApplicationExtendedSession applicationSession;
    return applicationSession;
}

DiagSession::DiagSession(SessionType id, uint8_t index) : fType(id), fId(index) {}

DiagSession& ApplicationDefaultSession::getTransitionResult(DiagSession::SessionType targetSession)
{
    return *this;
}

DiagReturnCode::Type
ApplicationDefaultSession::isTransitionPossible(DiagSession::SessionType targetSession)
{
    return DiagReturnCode::OK;
}

DiagSession& ApplicationExtendedSession::getTransitionResult(DiagSession::SessionType targetSession)
{
    return *this;
}

DiagReturnCode::Type
ApplicationExtendedSession::isTransitionPossible(DiagSession::SessionType targetSession)
{
    return DiagReturnCode::OK;
}

bool operator==(DiagSession const& x, DiagSession const& y) { return true; }

bool operator!=(DiagSession const& x, DiagSession const& y) { return false; }

void AbstractDiagJob::responseSent(IncomingDiagConnection& connection, ResponseSendResult result) {}

DiagReturnCode::Type AbstractDiagJob::execute(
    IncomingDiagConnection& connection, uint8_t const request[], uint16_t requestLength)
{
    DiagReturnCode::Type status = verify(request, requestLength);
    if (status == DiagReturnCode::OK)
    {
        if (fRequestLength > 0)
        {
            getDiagSessionManager().acceptedJob(
                connection,
                *this,
                request + (fRequestLength - fPrefixLength),
                requestLength - (fRequestLength - fPrefixLength));
        }
        status = process(
            connection,
            request + (fRequestLength - fPrefixLength),
            requestLength - (fRequestLength - fPrefixLength));
    }
    return status;
}

IDiagSessionManager* AbstractDiagJob::sfpSessionManager = 0;

void AbstractDiagJob::setDefaultDiagSessionManager(IDiagSessionManager& sessionManager)
{
    sfpSessionManager = &sessionManager;
}

IDiagSessionManager& AbstractDiagJob::getDiagSessionManager() { return *sfpSessionManager; }

IDiagSessionManager const& AbstractDiagJob::getDiagSessionManager() const
{
    return *sfpSessionManager;
}

IDiagAuthenticator const& AbstractDiagJob::getDiagAuthenticator() const
{
    static IDiagAuthenticator authenticator;
    return authenticator;
}

DiagReturnCode::Type AbstractDiagJob::process(
    IncomingDiagConnection& connection, uint8_t const request[], uint16_t requestLength)
{
    return DiagReturnCode::ISO_GENERAL_REJECT;
}

/*
 * class DiagJobRoot
 */
DiagJobRoot::DiagJobRoot() : AbstractDiagJob(nullptr, 0, 0)
{
    fDefaultDiagReturnCode = DiagReturnCode::ISO_SERVICE_NOT_SUPPORTED;
}

DiagJobRoot::~DiagJobRoot() {}

DiagReturnCode::Type DiagJobRoot::verify(uint8_t const request[], uint16_t requestLength)
{
    return DiagReturnCode::OK;
}

DiagReturnCode::Type DiagJobRoot::execute(
    IncomingDiagConnection& connection, uint8_t const request[], uint16_t requestLength)
{
    return DiagReturnCode::OK;
}

DiagReturnCode::Type
DiagJobRoot::verifySupplierIndication(uint8_t const* const request, uint16_t const requestLength)
{
    return DiagReturnCode::OK;
}

} // namespace uds
