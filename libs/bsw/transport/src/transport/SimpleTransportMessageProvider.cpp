// Copyright 2024 Accenture.

#include "transport/SimpleTransportMessageProvider.h"

namespace transport
{
// explicit
SimpleTransportMessageProvider::SimpleTransportMessageProvider(
    ITransportMessageProviderConfiguration& configuration)
: fConfiguration(configuration)
{}

ITransportMessageProvidingListener::ErrorCode SimpleTransportMessageProvider::getTransportMessage(
    uint8_t const /* srcBusId */,
    uint16_t /* sourceId */,
    uint16_t /* targetId */,
    uint16_t const size,
    ::estd::slice<uint8_t const> const& /* peek */,
    TransportMessage*& pTransportMessage)
{
    pTransportMessage = nullptr;
    if (size > static_cast<uint16_t>((fConfiguration.getBufferSize())))
    {
        return ITransportMessageProvider::ErrorCode::TPMSG_SIZE_TOO_LARGE;
    }
    if (!fConfiguration.empty())
    {
        pTransportMessage = &(fConfiguration.acquire());
    }
    return (pTransportMessage != nullptr)
               ? ITransportMessageProvider::ErrorCode::TPMSG_OK
               : ITransportMessageProvider::ErrorCode::TPMSG_NO_MSG_AVAILABLE;
}

void SimpleTransportMessageProvider::releaseTransportMessage(TransportMessage& transportMessage)
{
    fConfiguration.release(transportMessage);
}

// virtual
void SimpleTransportMessageProvider::dump() {}

} // namespace transport
