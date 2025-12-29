// Copyright 2025 Accenture.

#include "doip/common/DoIpTransportMessageProvidingListenerHelper.h"

#include "doip/common/DoIpConstants.h"

namespace doip
{
IDoIpTransportMessageProvidingListener::GetResult
DoIpTransportMessageProvidingListenerHelper::getTransportMessage(
    uint8_t const sourceBusId,
    uint16_t const sourceId,
    uint16_t const targetId,
    uint16_t const size,
    ::estd::slice<uint8_t const> const& peek,
    ::transport::TransportMessage*& transportMessage)
{
    if (_transportMessageProvidingListener != nullptr)
    {
        return _transportMessageProvidingListener->getTransportMessage(
            sourceBusId, sourceId, targetId, size, peek, transportMessage);
    }

    return createGetResult(_fallbackTransportMessageProvidingListener.getTransportMessage(
        sourceBusId, sourceId, targetId, size, peek, transportMessage));
}

void DoIpTransportMessageProvidingListenerHelper::releaseTransportMessage(
    ::transport::TransportMessage& transportMessage)
{
    if (_transportMessageProvidingListener != nullptr)
    {
        _transportMessageProvidingListener->releaseTransportMessage(transportMessage);
    }
    else
    {
        _fallbackTransportMessageProvidingListener.releaseTransportMessage(transportMessage);
    }
}

IDoIpTransportMessageProvidingListener::ReceiveResult
DoIpTransportMessageProvidingListenerHelper::messageReceived(
    uint8_t const sourceBusId,
    ::transport::TransportMessage& transportMessage,
    ::transport::ITransportMessageProcessedListener* const notificationListener)
{
    if (_transportMessageProvidingListener != nullptr)
    {
        return _transportMessageProvidingListener->messageReceived(
            sourceBusId, transportMessage, notificationListener);
    }

    return createReceiveResult(_fallbackTransportMessageProvidingListener.messageReceived(
        sourceBusId, transportMessage, notificationListener));
}

::transport::ITransportMessageProvider::ErrorCode
DoIpTransportMessageProvidingListenerHelper::getErrorCode(uint8_t const nackCode)
{
    switch (nackCode)
    {
        case DoIpConstants::DiagnosticMessageNackCodes::NACK_DIAG_INVALID_SOURCE_ADDRESS:
        {
            return ::transport::ITransportMessageProvider::ErrorCode::TPMSG_INVALID_SRC_ADDRESS;
        }
        case DoIpConstants::DiagnosticMessageNackCodes::NACK_DIAG_INVALID_TARGET_ADDRESS:
        case DoIpConstants::DiagnosticMessageNackCodes::NACK_DIAG_TARGET_UNREACHABLE:
        {
            return ::transport::ITransportMessageProvider::ErrorCode::TPMSG_INVALID_TGT_ADDRESS;
        }
        case DoIpConstants::DiagnosticMessageNackCodes::NACK_DIAG_DIAGNOSTIC_MESSAGE_TOO_LARGE:
        {
            return ::transport::ITransportMessageProvider::ErrorCode::TPMSG_SIZE_TOO_LARGE;
        }
        case DoIpConstants::DiagnosticMessageNackCodes::NACK_DIAG_OUT_OF_MEMORY:
        {
            return ::transport::ITransportMessageProvider::ErrorCode::TPMSG_NO_MSG_AVAILABLE;
        }
        default:
        {
            return ::transport::ITransportMessageProvider::ErrorCode::TPMSG_NOT_RESPONSIBLE;
        }
    }
}

::transport::ITransportMessageListener::ReceiveResult DoIpTransportMessageProvidingListenerHelper::
    DoIpTransportMessageProvidingListenerHelper::getReceiveResult(uint8_t const /*nackCode*/)
{
    return ::transport::ITransportMessageListener::ReceiveResult::RECEIVED_ERROR;
}

uint8_t DoIpTransportMessageProvidingListenerHelper::getDiagnosticNackCode(
    ::transport::ITransportMessageProvider::ErrorCode const errorCode)
{
    switch (errorCode)
    {
        case ::transport::ITransportMessageProvider::ErrorCode::TPMSG_INVALID_SRC_ADDRESS:
        {
            return DoIpConstants::DiagnosticMessageNackCodes::NACK_DIAG_INVALID_SOURCE_ADDRESS;
        }
        case ::transport::ITransportMessageProvider::ErrorCode::TPMSG_INVALID_TGT_ADDRESS:
        {
            return DoIpConstants::DiagnosticMessageNackCodes::NACK_DIAG_INVALID_TARGET_ADDRESS;
        }
        case ::transport::ITransportMessageProvider::ErrorCode::TPMSG_SIZE_TOO_LARGE:
        {
            return DoIpConstants::DiagnosticMessageNackCodes::
                NACK_DIAG_DIAGNOSTIC_MESSAGE_TOO_LARGE;
        }
        case ::transport::ITransportMessageProvider::ErrorCode::TPMSG_NO_MSG_AVAILABLE:
        {
            return DoIpConstants::DiagnosticMessageNackCodes::NACK_DIAG_OUT_OF_MEMORY;
        }
        default:
        {
            return DoIpConstants::DiagnosticMessageNackCodes::NACK_DIAG_UNKNOWN_NETWORK;
        }
    }
}

uint8_t DoIpTransportMessageProvidingListenerHelper::getDiagnosticNackCode(
    ::transport::ITransportMessageListener::ReceiveResult const /*receiveResult*/)
{
    return DoIpConstants::DiagnosticMessageNackCodes::NACK_DIAG_TRANSPORT_PROTOCOL_ERROR;
}

} // namespace doip
