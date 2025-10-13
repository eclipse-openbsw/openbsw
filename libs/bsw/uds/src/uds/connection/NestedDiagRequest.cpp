// Copyright 2024 Accenture.

#include "uds/connection/NestedDiagRequest.h"

#include <etl/memory.h>
#include <etl/span.h>

namespace uds
{

NestedDiagRequest::NestedDiagRequest(uint8_t const prefixLength)
: senderJob(nullptr)
, pendingResponseSender(nullptr)
, prefixLength(prefixLength)
, isPendingSent(false)
, responseCode(DiagReturnCode::OK)
, _messageBuffer()
, _storedRequestLength(0)
, _numIdentifiers(0)
, _numPrefixIdentifiers(0)
{}

void NestedDiagRequest::init(
    AbstractDiagJob& sender,
    ::etl::span<uint8_t> const& messageBuffer,
    ::etl::span<uint8_t const> const& request)
{
    senderJob             = &sender;
    pendingResponseSender = nullptr;
    _messageBuffer        = messageBuffer;
    _storedRequestLength  = getStoredRequestLength(request);
    _responseLength       = 0U;
    _numIdentifiers       = 0U;
    _numPrefixIdentifiers = 0U;
    isPendingSent         = false;
    responseCode          = DiagReturnCode::OK;
    if (_storedRequestLength > 0U)
    {
        storeRequest(
            request,
            _messageBuffer.subspan(
                static_cast<size_t>(_messageBuffer.size() - _storedRequestLength)));
    }
}

bool NestedDiagRequest::prepareNextRequest()
{
    if (responseCode == DiagReturnCode::OK)
    {
        _nestedRequest = prepareNestedRequest(_messageBuffer.subspan(
            static_cast<size_t>(_messageBuffer.size() - _storedRequestLength)));
    }
    return (responseCode == DiagReturnCode::OK) && (_nestedRequest.size() > 0U);
}

DiagReturnCode::Type NestedDiagRequest::processNextRequest(IncomingDiagConnection& connection)
{
    _numIdentifiers       = 0U;
    _numPrefixIdentifiers = 0U;
    return processNestedRequest(
        connection, _nestedRequest.data(), static_cast<uint16_t>(_nestedRequest.size()));
}

void NestedDiagRequest::handleNegativeResponseCode(DiagReturnCode::Type const responseCode)
{
    handleNestedResponseCode(responseCode);
}

void NestedDiagRequest::handleResponseOverflow() { handleOverflow(); }

::etl::span<uint8_t> NestedDiagRequest::getResponseBuffer()
{
    if (_numIdentifiers >= prefixLength)
    {
        _numPrefixIdentifiers = _numIdentifiers - prefixLength;
        (void)::etl::copy(
            _nestedRequest.subspan(prefixLength, static_cast<size_t>(_numPrefixIdentifiers)),
            _messageBuffer);
    }
    return _messageBuffer.subspan(
        static_cast<size_t>(_numPrefixIdentifiers),
        static_cast<size_t>(_messageBuffer.size() - _storedRequestLength - _numPrefixIdentifiers));
}

uint16_t NestedDiagRequest::getMaxNestedResponseLength() const
{
    return static_cast<uint16_t>(_messageBuffer.size()) - _storedRequestLength
           - static_cast<uint16_t>(_numIdentifiers);
}

void NestedDiagRequest::setNestedResponseLength(uint16_t responseLength)
{
    responseLength += _numPrefixIdentifiers;
    _messageBuffer = _messageBuffer.subspan(static_cast<size_t>(responseLength));
    _responseLength += responseLength;
    handleNestedResponseCode(DiagReturnCode::OK);
}

void NestedDiagRequest::addIdentifier() { ++_numIdentifiers; }

uint8_t NestedDiagRequest::getIdentifier(uint16_t const idx) const
{
    if (idx < _numIdentifiers)
    {
        return _nestedRequest[static_cast<size_t>(idx)];
    }
    else
    {
        return 0U;
    }
}

uint16_t NestedDiagRequest::getStoredRequestLength(::etl::span<uint8_t const> const& request) const
{
    return static_cast<uint16_t>(request.size());
}

void NestedDiagRequest::storeRequest(
    ::etl::span<uint8_t const> const& request, ::etl::span<uint8_t> dest) const
{
    (void)::etl::copy(request, dest);
}

void NestedDiagRequest::handleOverflow() { responseCode = DiagReturnCode::ISO_RESPONSE_TOO_LONG; }

void NestedDiagRequest::handleNestedResponseCode(DiagReturnCode::Type const nestedResponseCode)
{
    responseCode = nestedResponseCode;
}

::etl::span<uint8_t const> NestedDiagRequest::consumeStoredRequest(uint16_t const consumedLength)
{
    if (_storedRequestLength < consumedLength)
    {
        return ::etl::span<uint8_t const>();
    }
    _storedRequestLength -= consumedLength;
    return ::etl::span<uint8_t const>(_messageBuffer.subspan(
        _messageBuffer.size()
            - (static_cast<size_t>(_storedRequestLength) + static_cast<size_t>(consumedLength)),
        static_cast<size_t>(consumedLength)));
}

} // namespace uds
