/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

/**
 * \ingroup doip
 */
#pragma once

#include "doip/common/IDoIpTransportMessageProvidingListener.h"

#include <transport/ITransportMessageProvidingListener.h>

namespace doip
{
/**
 * This helper class simplifies using a DoIP specific message providing listener as maybe needed
 * in a DoIP transport layer. An optional DoIP specific message provider may be initialized. If this
 * specialized interface is not available this helper forwards the calls to the mandatory generic
 * fall-back interface and maps the return values to reasonable DoIP NACK codes.
 */
class DoIpTransportMessageProvidingListenerHelper : public IDoIpTransportMessageProvidingListener
{
public:
    /**
     * Constructor.
     * \param fallbackTransportMessageProvidingListener reference to fall-back transport message
     * providing listener interface that should be used if no DoIP specific transport message
     * providing interface is initialized
     */
    explicit DoIpTransportMessageProvidingListenerHelper(
        ::transport::ITransportMessageProvidingListener& fallbackTransportMessageProvidingListener);

    /**
     * Set the interface to wrap.
     * \param transportMessageProvidingListener pointer to interface
     */
    void setTransportMessageProvidingListener(
        IDoIpTransportMessageProvidingListener* transportMessageProvidingListener);

    /**
     * \see IDoIpTransportMessageProvidingListener::getTransportMessage()
     */
    GetResult getTransportMessage(
        uint8_t sourceBusId,
        uint16_t sourceId,
        uint16_t targetId,
        uint16_t size,
        ::etl::span<uint8_t const> const& peek,
        ::transport::TransportMessage*& transportMessage) override;

    /**
     * \see IDoIpTransportMessageProvidingListener::releaseTransportMessage()
     */
    void releaseTransportMessage(::transport::TransportMessage& transportMessage) override;

    /**
     * \see IDoIpTransportMessageProvidingListener::messageReceived()
     */
    ReceiveResult messageReceived(
        uint8_t sourceBusId,
        ::transport::TransportMessage& transportMessage,
        ::transport::ITransportMessageProcessedListener* notificationListener) override;

    /**
     * Create default GetResult from message provider error code.
     * \param errorCode error code to create result from
     * \return result holding both diagnostic NACK code and corresponding transport error code
     */
    static GetResult createGetResult(::transport::ITransportMessageProvider::ErrorCode errorCode);

    /**
     * Create default GetResult from diagnostic nack code.
     * \param nackCode diagnostic NACK code to create result from
     * \return result holding both diagnostic NACK code and corresponding transport error code
     */
    static GetResult createGetResult(uint8_t nackCode);

    /**
     * Create default ReceiveResult from message listener receive result.
     * \param receiveResult receive result as returned from message listener
     * \return result holding both diagnostic NACK code and corresponding transport receive result
     */
    static ReceiveResult
    createReceiveResult(::transport::ITransportMessageListener::ReceiveResult receiveResult);

    /**
     * Create default ReceiveResult from diagnostic nack code
     * \param nackCode diagnostic NACK code to create result from
     * \return result holding both diagnostic NACK code and corresponding transport receive result
     */
    static ReceiveResult createReceiveResult(uint8_t nackCode);

    /**
     * Get transport message provider error code from diagnostic NACK code.
     * \param nackCode diagnostic nack code to get message provider error for
     * \return default transport message error code for nackCode
     */
    static ::transport::ITransportMessageProvider::ErrorCode getErrorCode(uint8_t nackCode);

    /**
     * Get receive result from diagnostic NACK code.
     * \param nackCode diagnostic nack code to get message provider error for
     * \return default transport message error code for nackCode
     */
    static ::transport::ITransportMessageListener::ReceiveResult getReceiveResult(uint8_t nackCode);

    /**
     * Get default diagnostic NACK code for message provider error code.
     * \param errorCode error code to create result from
     * \return default diagnostic NACK code for error code
     */
    static uint8_t
    getDiagnosticNackCode(::transport::ITransportMessageProvider::ErrorCode errorCode);

    /**
     * Get default diagnostic NACK code for message listener receive result.
     * \param receiveResult receive result as returned from message listener
     * \return default diagnostic NACK code for receive result
     */
    static uint8_t
    getDiagnosticNackCode(::transport::ITransportMessageListener::ReceiveResult receiveResult);

private:
    ::transport::ITransportMessageProvidingListener& _fallbackTransportMessageProvidingListener;
    IDoIpTransportMessageProvidingListener* _transportMessageProvidingListener;
};

/**
 * Inline implementations.
 */
inline DoIpTransportMessageProvidingListenerHelper::DoIpTransportMessageProvidingListenerHelper(
    ::transport::ITransportMessageProvidingListener& fallbackTransportMessageProvidingListener)
: _fallbackTransportMessageProvidingListener(fallbackTransportMessageProvidingListener)
, _transportMessageProvidingListener(nullptr)
{}

inline void DoIpTransportMessageProvidingListenerHelper::setTransportMessageProvidingListener(
    IDoIpTransportMessageProvidingListener* const transportMessageProvidingListener)
{
    _transportMessageProvidingListener = transportMessageProvidingListener;
}

inline IDoIpTransportMessageProvidingListener::GetResult
DoIpTransportMessageProvidingListenerHelper::createGetResult(
    ::transport::ITransportMessageProvider::ErrorCode const errorCode)
{
    return IDoIpTransportMessageProvidingListener::GetResult(
        errorCode, getDiagnosticNackCode(errorCode));
}

inline IDoIpTransportMessageProvidingListener::GetResult
DoIpTransportMessageProvidingListenerHelper::createGetResult(uint8_t const nackCode)
{
    return IDoIpTransportMessageProvidingListener::GetResult(getErrorCode(nackCode), nackCode);
}

inline IDoIpTransportMessageProvidingListener::ReceiveResult
DoIpTransportMessageProvidingListenerHelper::createReceiveResult(
    ::transport::ITransportMessageListener::ReceiveResult const receiveResult)
{
    return IDoIpTransportMessageProvidingListener::ReceiveResult(
        receiveResult, getDiagnosticNackCode(receiveResult));
}

inline IDoIpTransportMessageProvidingListener::ReceiveResult
DoIpTransportMessageProvidingListenerHelper::createReceiveResult(uint8_t const nackCode)
{
    return IDoIpTransportMessageProvidingListener::ReceiveResult(
        getReceiveResult(nackCode), nackCode);
}

} // namespace doip
