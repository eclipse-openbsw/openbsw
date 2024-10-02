// Copyright 2024 Accenture.

#include "uds/connection/IncomingDiagConnectionMock.h"

#include "StubMock.h"

namespace uds
{

void IncomingDiagConnection::addIdentifier()
{
    if (!IncomingDiagConnectionMockHelper::instance().isStub())
    {
        IncomingDiagConnectionMockHelper::instance().addIdentifier();
    }
}

void IncomingDiagConnection::transportMessageProcessed(
    transport::TransportMessage& transportMessage, ProcessingResult result)
{
    if (!IncomingDiagConnectionMockHelper::instance().isStub())
    {
        IncomingDiagConnectionMockHelper::instance().transportMessageProcessed(
            transportMessage, result);
    }
}

void IncomingDiagConnection::triggerNextNestedRequest()
{
    if (!IncomingDiagConnectionMockHelper::instance().isStub())
    {
        IncomingDiagConnectionMockHelper::instance().triggerNextNestedRequest();
    }
}

void IncomingDiagConnection::asyncSendNegativeResponse(
    uint8_t responseCode, AbstractDiagJob* pSender)
{
    if (!IncomingDiagConnectionMockHelper::instance().isStub())
    {
        IncomingDiagConnectionMockHelper::instance().asyncSendNegativeResponse(
            responseCode, pSender);
    }
}

void IncomingDiagConnection::asyncSendPositiveResponse(
    uint16_t responseCode, AbstractDiagJob* pSender)
{
    if (!IncomingDiagConnectionMockHelper::instance().isStub())
    {
        IncomingDiagConnectionMockHelper::instance().asyncSendPositiveResponse(
            responseCode, pSender);
    }
}

void IncomingDiagConnection::asyncTransportMessageProcessed(
    transport::TransportMessage* pTransportMessage, ProcessingResult status)
{
    if (!IncomingDiagConnectionMockHelper::instance().isStub())
    {
        IncomingDiagConnectionMockHelper::instance().asyncTransportMessageProcessed(
            pTransportMessage, status);
    }
}

void IncomingDiagConnection::expired(::async::RunnableType const& timeout)
{
    if (!IncomingDiagConnectionMockHelper::instance().isStub())
    {
        IncomingDiagConnectionMockHelper::instance().expired(timeout, actions);
    }
}

PositiveResponse& IncomingDiagConnection::releaseRequestGetResponse()
{
    if (IncomingDiagConnectionMockHelper::instance().isStub())
    {
        return fPositiveResponse;
    }
    return IncomingDiagConnectionMockHelper::instance().releaseRequestGetResponse();
}

::uds::ErrorCode
IncomingDiagConnection::sendNegativeResponse(uint8_t responseCode, AbstractDiagJob& sender)
{
    if (IncomingDiagConnectionMockHelper::instance().isStub())
    {
        return OK;
    }
    return IncomingDiagConnectionMockHelper::instance().sendNegativeResponse(responseCode, sender);
}

uint8_t IncomingDiagConnection::getIdentifier(uint16_t idx) const
{
    if (IncomingDiagConnectionMockHelper::instance().isStub())
    {
        if (idx >= fIdentifiers.size())
        {
            return 0U;
        }
        return fIdentifiers[idx];
    }
    return IncomingDiagConnectionMockHelper::instance().getIdentifier(idx);
}

} // namespace uds
