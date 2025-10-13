// Copyright 2024 Accenture.

#include "transport/TransportMessageProvidingListenerMock.h"

#include "transport/ITransportMessageProcessedListener.h"
#include "transport/TransportMessage.h"

using namespace ::testing;
using namespace ::std;

namespace transport
{
TransportMessageProvidingListenerMock::TransportMessageProvidingListenerMock(bool defaultImpl)
{
    if (defaultImpl)
    {
        ON_CALL(*this, getTransportMessage(_, _, _, _, _, _))
            .WillByDefault(Invoke(
                this, &TransportMessageProvidingListenerMock::getTransportMessageImplementation));
        ON_CALL(*this, messageReceived(_, _, _))
            .WillByDefault(Invoke(
                this, &TransportMessageProvidingListenerMock::messageReceivedImplementation));
        ON_CALL(*this, releaseTransportMessage(_))
            .WillByDefault(Invoke(
                this,
                &TransportMessageProvidingListenerMock::releaseTransportMessageImplementation));
        ON_CALL(*this, refuseSourceId(_))
            .WillByDefault(Invoke(
                this, &TransportMessageProvidingListenerMock::refuseSourceAddressImplementation));
        ON_CALL(*this, refuseTargetId(_))
            .WillByDefault(Invoke(
                this, &TransportMessageProvidingListenerMock::refuseTargetAddressImplementation));
    }
}

ITransportMessageProvider::ErrorCode
TransportMessageProvidingListenerMock::getTransportMessageImplementation(
    uint8_t /* srcBusId */,
    uint16_t sourceAddress,
    uint16_t targetAddress,
    uint16_t size,
    ::etl::span<uint8_t const> const& /* peek */,
    TransportMessage*& pTransportMessage)
{
    if (fRefusedSourceIds.count(sourceAddress) > 0)
    {
        pTransportMessage = 0L;
        return ITransportMessageProvidingListener::ErrorCode::TPMSG_INVALID_SRC_ADDRESS;
    }
    if (fRefusedTargetIds.count(targetAddress) > 0)
    {
        pTransportMessage = 0L;
        return ITransportMessageProvidingListener::ErrorCode::TPMSG_INVALID_TGT_ADDRESS;
    }
    pTransportMessage = new TransportMessage();
    pTransportMessage->init(new uint8_t[size], size);
    return ITransportMessageProvider::ErrorCode::TPMSG_OK;
}

void TransportMessageProvidingListenerMock::releaseTransportMessageImplementation(
    TransportMessage& transportMessage)
{
    TransportMessage* pTransportMessage = &transportMessage;
    uint8_t* pBuffer                    = transportMessage.getPayload();
    delete[] pBuffer;
    delete pTransportMessage;
}

ITransportMessageListener::ReceiveResult
TransportMessageProvidingListenerMock::messageReceivedImplementation(
    uint8_t /* sourceBusId */,
    TransportMessage& transportMessage,
    ITransportMessageProcessedListener* pNotificationListener)
{
    if (pNotificationListener)
    {
        pNotificationListener->transportMessageProcessed(
            transportMessage,
            ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR);
    }
    return ITransportMessageListener::ReceiveResult::RECEIVED_NO_ERROR;
}

void TransportMessageProvidingListenerMock::refuseSourceAddressImplementation(uint8_t sourceAddress)
{
    fRefusedSourceIds.insert(sourceAddress);
}

void TransportMessageProvidingListenerMock::refuseTargetAddressImplementation(uint8_t targetAddress)
{
    fRefusedTargetIds.insert(targetAddress);
}

} // namespace transport
