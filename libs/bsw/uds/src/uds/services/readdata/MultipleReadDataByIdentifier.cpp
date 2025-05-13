// Copyright 2024 Accenture.

#include "uds/services/readdata/MultipleReadDataByIdentifier.h"

#include "uds/async/AsyncDiagHelper.h"
#include "uds/connection/IncomingDiagConnection.h"
#include "uds/session/DiagSession.h"

#include <etl/memory.h>
#include <transport/TransportMessage.h>

namespace uds
{

uint8_t const thisimplementedRequest[] = {ServiceId::READ_DATA_BY_IDENTIFIER};

MultipleReadDataByIdentifier::MultipleReadDataByIdentifier(IAsyncDiagHelper& asyncHelper)
: AbstractDiagJob(&thisimplementedRequest[0], 1U, 0U, DiagSession::ALL_SESSIONS())
, NestedDiagRequest(1U)
, fAsyncJobHelper(asyncHelper, *this)
, fFirstJob(*this)
, fGetDidLimit()
, fCheckResponse()
, fBuffer()
, fCombinedResponseCode(DiagReturnCode::ISO_REQUEST_OUT_OF_RANGE)
{
    fBuffer[0U] = ServiceId::READ_DATA_BY_IDENTIFIER;
    setDefaultDiagReturnCode(DiagReturnCode::ISO_REQUEST_OUT_OF_RANGE);
    setCheckResponse(CheckResponseType());
}

MultipleReadDataByIdentifier::MultipleReadDataByIdentifier(
    IAsyncDiagHelper& asyncHelper, AbstractDiagJob& firstJob)
: AbstractDiagJob(&thisimplementedRequest[0], 1U, 0U, DiagSession::ALL_SESSIONS())
, NestedDiagRequest(1U)
, fAsyncJobHelper(asyncHelper, *this)
, fFirstJob(firstJob)
, fGetDidLimit()
, fCheckResponse()
, fBuffer()
, fCombinedResponseCode(DiagReturnCode::ISO_REQUEST_OUT_OF_RANGE)
{
    fBuffer[0U] = ServiceId::READ_DATA_BY_IDENTIFIER;
    setDefaultDiagReturnCode(DiagReturnCode::ISO_REQUEST_OUT_OF_RANGE);
    setCheckResponse(CheckResponseType());
}

void MultipleReadDataByIdentifier::setGetDidLimit(GetDidLimitType const getDidLimit)
{
    fGetDidLimit = getDidLimit;
}

void MultipleReadDataByIdentifier::setCheckResponse(CheckResponseType const checkResponse)
{
    if (checkResponse.is_valid())
    {
        fCheckResponse = checkResponse;
    }
    else
    {
        fCheckResponse = CheckResponseType::create<
            MultipleReadDataByIdentifier,
            &MultipleReadDataByIdentifier::defaultCheckResponse>(*this);
    }
}

DiagReturnCode::Type
MultipleReadDataByIdentifier::verify(uint8_t const* const request, uint16_t const requestLength)
{
    if (request[0] == ServiceId::READ_DATA_BY_IDENTIFIER)
    {
        if ((requestLength >= 3U) && (((requestLength - 1U) % 2U) == 0U))
        {
            return DiagReturnCode::OK;
        }
        else
        {
            return DiagReturnCode::ISO_INVALID_FORMAT;
        }
    }
    else
    {
        return DiagReturnCode::NOT_RESPONSIBLE;
    }
}

DiagReturnCode::Type MultipleReadDataByIdentifier::process(
    IncomingDiagConnection& connection, uint8_t const* const request, uint16_t const requestLength)
{
    if (requestLength == 2U)
    {
        return AbstractDiagJob::process(connection, request, requestLength);
    }
    else if (fGetDidLimit.is_valid())
    {
        uint8_t const didLimit = fGetDidLimit(*connection.fpRequestMessage);
        if ((didLimit > 0U) && ((requestLength / 2U) > didLimit))
        {
            return DiagReturnCode::ISO_INVALID_FORMAT;
        }
    }
    else
    {
        // nothing to do
    }
    if (fAsyncJobHelper.hasPendingAsyncRequest())
    {
        return fAsyncJobHelper.enqueueRequest(connection, request, requestLength);
    }
    fCombinedResponseCode = DiagReturnCode::ISO_REQUEST_OUT_OF_RANGE;
    fAsyncJobHelper.startAsyncRequest(connection);
    return connection.startNestedRequest(*this, *this, request, requestLength);
}

void MultipleReadDataByIdentifier::responseSent(
    IncomingDiagConnection& connection, ResponseSendResult const result)
{
    AbstractDiagJob::responseSent(connection, result);
    fAsyncJobHelper.endAsyncRequest();
}

::etl::span<uint8_t const>
MultipleReadDataByIdentifier::prepareNestedRequest(::etl::span<uint8_t const> const& storedRequest)
{
    if (storedRequest.size() >= 2U)
    {
        ::etl::span<uint8_t const> src{consumeStoredRequest(2U)};
        (void)::etl::mem_copy<uint8_t>(src.cbegin(), src.size(), fBuffer.begin() + 1);
        return fBuffer;
    }
    else
    {
        setResponseCode(fCombinedResponseCode);
        return {};
    }
}

DiagReturnCode::Type MultipleReadDataByIdentifier::processNestedRequest(
    IncomingDiagConnection& connection, uint8_t const* const request, uint16_t const requestLength)
{
    DiagReturnCode::Type responseCode = DiagReturnCode::NOT_RESPONSIBLE;
    AbstractDiagJob* currentJob       = &fFirstJob;
    while ((responseCode == DiagReturnCode::NOT_RESPONSIBLE) && (currentJob != nullptr))
    {
        responseCode = currentJob->execute(connection, request, requestLength);
        currentJob   = getNextJob(*currentJob);
    }
    return responseCode;
}

void MultipleReadDataByIdentifier::handleNestedResponseCode(DiagReturnCode::Type const responseCode)
{
    if (!fCheckResponse(responseCode, fCombinedResponseCode))
    {
        setResponseCode(fCombinedResponseCode);
    }
}

bool MultipleReadDataByIdentifier::defaultCheckResponse(
    DiagReturnCode::Type const responseCode, DiagReturnCode::Type& combinedResponse)
{
    if ((responseCode == DiagReturnCode::NOT_RESPONSIBLE)
        || (responseCode == DiagReturnCode::ISO_REQUEST_OUT_OF_RANGE))
    {
        return true;
    }
    else
    {
        combinedResponse = responseCode;
        return responseCode == DiagReturnCode::OK;
    }
}

} // namespace uds
