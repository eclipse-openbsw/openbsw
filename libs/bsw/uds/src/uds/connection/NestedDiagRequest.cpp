// Copyright 2024 Accenture.

#include "uds/connection/NestedDiagRequest.h"

#include <estd/memory.h>

namespace uds
{

NestedDiagRequest::NestedDiagRequest(uint8_t const prefixLength)
: fSender(nullptr)
, fPendingResponseSender(nullptr)
, fMessageBuffer()
, fNestedRequest()
, fStoredRequestLength(0)
, fResponseLength(0)
, fPrefixLength(prefixLength)
, fNumIdentifiers(0)
, fNumPrefixIdentifiers(0)
, fIsPendingSent(false)
, fResponseCode(DiagReturnCode::OK)
{}

void NestedDiagRequest::init(
    AbstractDiagJob& sender,
    ::estd::slice<uint8_t> const& messageBuffer,
    ::estd::slice<uint8_t const> const& request)
{
    fSender                = &sender;
    fPendingResponseSender = nullptr;
    fMessageBuffer         = messageBuffer;
    fStoredRequestLength   = getStoredRequestLength(request);
    fResponseLength        = 0U;
    fNumIdentifiers        = 0U;
    fNumPrefixIdentifiers  = 0U;
    fIsPendingSent         = false;
    fResponseCode          = DiagReturnCode::OK;
    if (fStoredRequestLength > 0U)
    {
        storeRequest(
            request,
            fMessageBuffer.offset(
                static_cast<size_t>(fMessageBuffer.size() - fStoredRequestLength)));
    }
}

bool NestedDiagRequest::prepareNextRequest()
{
    if (fResponseCode == DiagReturnCode::OK)
    {
        fNestedRequest = prepareNestedRequest(fMessageBuffer.offset(
            static_cast<size_t>(fMessageBuffer.size() - fStoredRequestLength)));
    }
    return (fResponseCode == DiagReturnCode::OK) && (fNestedRequest.size() > 0U);
}

DiagReturnCode::Type NestedDiagRequest::processNextRequest(IncomingDiagConnection& connection)
{
    fNumIdentifiers       = 0U;
    fNumPrefixIdentifiers = 0U;
    return processNestedRequest(
        connection, fNestedRequest.data(), static_cast<uint16_t>(fNestedRequest.size()));
}

void NestedDiagRequest::handleNegativeResponseCode(DiagReturnCode::Type const responseCode)
{
    handleNestedResponseCode(responseCode);
}

void NestedDiagRequest::handleResponseOverflow() { handleOverflow(); }

::estd::slice<uint8_t> NestedDiagRequest::getResponseBuffer()
{
    if (fNumIdentifiers >= fPrefixLength)
    {
        fNumPrefixIdentifiers = fNumIdentifiers - fPrefixLength;
        (void)::estd::memory::copy(
            fMessageBuffer,
            fNestedRequest.offset(static_cast<size_t>(fPrefixLength))
                .subslice(static_cast<size_t>(fNumPrefixIdentifiers)));
    }
    return fMessageBuffer.offset(static_cast<size_t>(fNumPrefixIdentifiers))
        .subslice(static_cast<size_t>(
            fMessageBuffer.size() - fStoredRequestLength - fNumPrefixIdentifiers));
}

uint16_t NestedDiagRequest::getMaxNestedResponseLength() const
{
    return static_cast<uint16_t>(fMessageBuffer.size()) - fStoredRequestLength
           - static_cast<uint16_t>(fNumIdentifiers);
}

void NestedDiagRequest::setNestedResponseLength(uint16_t responseLength)
{
    responseLength += fNumPrefixIdentifiers;
    fMessageBuffer = fMessageBuffer.offset(static_cast<size_t>(responseLength));
    fResponseLength += responseLength;
    handleNestedResponseCode(DiagReturnCode::OK);
}

void NestedDiagRequest::addIdentifier() { ++fNumIdentifiers; }

uint8_t NestedDiagRequest::getIdentifier(uint16_t const idx) const
{
    if (idx < fNumIdentifiers)
    {
        return fNestedRequest[static_cast<size_t>(idx)];
    }
    else
    {
        return 0U;
    }
}

uint16_t
NestedDiagRequest::getStoredRequestLength(::estd::slice<uint8_t const> const& request) const
{
    return static_cast<uint16_t>(request.size());
}

void NestedDiagRequest::storeRequest(
    ::estd::slice<uint8_t const> const& request, ::estd::slice<uint8_t> dest) const
{
    (void)::estd::memory::copy(dest, request);
}

void NestedDiagRequest::handleOverflow() { setResponseCode(DiagReturnCode::ISO_RESPONSE_TOO_LONG); }

void NestedDiagRequest::handleNestedResponseCode(DiagReturnCode::Type const responseCode)
{
    setResponseCode(responseCode);
}

::estd::slice<uint8_t const> NestedDiagRequest::consumeStoredRequest(uint16_t const consumedLength)
{
    if (fStoredRequestLength < consumedLength)
    {
        return ::estd::slice<uint8_t const>();
    }
    fStoredRequestLength -= consumedLength;
    return ::estd::slice<uint8_t const>(
        fMessageBuffer
            .offset(
                fMessageBuffer.size()
                - (static_cast<size_t>(fStoredRequestLength) + static_cast<size_t>(consumedLength)))
            .subslice(static_cast<size_t>(consumedLength)));
}

} // namespace uds
