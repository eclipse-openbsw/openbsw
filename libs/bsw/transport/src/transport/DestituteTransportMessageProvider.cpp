// Copyright 2024 Accenture.

/**
 * Contains
 * \file
 * \ingroup
 */

#include "transport/DestituteTransportMessageProvider.h"

namespace transport
{
DestituteTransportMessageProvider::DestituteTransportMessageProvider() = default;

// virtual
ITransportMessageProvidingListener::ErrorCode
DestituteTransportMessageProvider::getTransportMessage(
    uint8_t /* srcBusId */,
    uint16_t /* sourceId */,
    uint16_t /* targetId */,
    uint16_t /* size */,
    ::estd::slice<uint8_t const> const& /* peek */,
    TransportMessage*& pTransportMessage)
{
    pTransportMessage = nullptr;
    return ITransportMessageProvidingListener::ErrorCode::TPMSG_NOT_RESPONSIBLE;
}

// virtual
void DestituteTransportMessageProvider::releaseTransportMessage(
    TransportMessage& /* transportMessage */)
{}

// virtual
void DestituteTransportMessageProvider::dump() {}

} // namespace transport
