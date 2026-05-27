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

#include "doip/common/DoIpConstants.h"
#include "doip/common/DoIpHeader.h"
#include "doip/common/DoIpStaticPayloadSendJob.h"
#include "doip/common/DoIpTransportMessageSendJob.h"
#include "doip/common/IDoIpTransportMessageProvidingListener.h"
#include "doip/server/IDoIpServerMessageHandler.h"

#include <common/busid/BusId.h>
#include <etl/algorithm.h>
#include <etl/ipool.h>
#include <etl/span.h>

namespace doip
{
class DoIpServerTransportConnectionConfig;

/**
 * Class that handles transport message sending and reception.
 */
class DoIpServerTransportMessageHandler
: public IDoIpServerMessageHandler
, private IDoIpSendJobCallback<DoIpTransportMessageSendJob>
{
public:
    /**
     * Constructor.
     * \param protocolVersion doip protocol version used for all communication
     * \param diagnosticSendJobBlockPool reference to block pool for transport message send jobs
     * \param protocolSendJobBlockPool reference to block pool for protocol send jobs
     * \param config reference to configuration data for transport connections
     */
    DoIpServerTransportMessageHandler(
        DoIpConstants::ProtocolVersion protocolVersion,
        ::etl::ipool& diagnosticSendJobBlockPool,
        ::etl::ipool& protocolSendJobBlockPool,
        DoIpServerTransportConnectionConfig const& config);

    /**
     * Initiate sending of a transport message.
     * \param transportMessage reference to transport message to send
     * \param pNotificationListener pointer to notification listener that will
     *        be called if message has been sent. If this function returns true
     *        a given notification listener will always be called.
     * \return true if all preconditions for sending are satisfied.
     */
    bool send(
        ::transport::TransportMessage& transportMessage,
        ::transport::ITransportMessageProcessedListener* pNotificationListener);

    void connectionOpened(IDoIpServerConnection& connection) override;

    void routingActive() override;

    void connectionClosed() override;

    bool headerReceived(DoIpHeader const& header) override;

    // Send jobs declarations
    static constexpr size_t STATIC_PAYLOAD_SENDJOB_SIZE = 10U;
    using StaticPayloadSendJobType = declare::DoIpStaticPayloadSendJob<STATIC_PAYLOAD_SENDJOB_SIZE>;

private:
    static constexpr size_t PEEK_MAX_SIZE    = 4U;
    static constexpr size_t ACK_PAYLOAD_SIZE = 5U;
    static constexpr size_t PAYLOAD_PREFIX_BUFFER_SIZE
        = ::etl::max(PEEK_MAX_SIZE, ACK_PAYLOAD_SIZE);

    //  Helper struct to persist data across callback.
    struct PayloadPrefixContext
    {
        uint16_t sourceAddress;
        uint16_t targetAddress;
        uint8_t payloadPrefixBuffer[PAYLOAD_PREFIX_BUFFER_SIZE];
    };

    DoIpTransportMessageSendJob* allocateJob(
        ::transport::TransportMessage& transportMessage,
        ::transport::ITransportMessageProcessedListener* pNotificationListener);
    void releaseSendJob(DoIpTransportMessageSendJob& sendJob, bool success) override;

    IDoIpTransportMessageProvidingListener::GetResult getTpMessageAndReceiveDiagnosticUserData(
        uint16_t sourceAddress, uint16_t targetAddress, ::etl::span<uint8_t const> payloadPrefix);
    void diagnosticMessageLogicalAddressInfoReceived(::etl::span<uint8_t const> logicalAddressInfo);
    void diagnosticMessagePayloadPrefixDataReceived(::etl::span<uint8_t const> payloadPrefix);
    void diagnosticMessageUserDataReceived(::etl::span<uint8_t const> remainingPayload);
    void diagnosticMessageUserDataPrefixReceivedNack(
        uint8_t const nackCode,
        bool const closeAfterSend,
        ::etl::span<uint8_t const> const payload);

    StaticPayloadSendJobType* queueDiagnosticAck(
        uint16_t payloadType,
        uint16_t sourceAddress,
        uint16_t targetAddress,
        uint8_t responseCode,
        bool closeAfterSend,
        ::etl::span<uint8_t const> receivedMessageData);

    void releaseSendJobAndClose(DoIpStaticPayloadSendJob& sendJob, bool success);
    void releaseSendJob(DoIpStaticPayloadSendJob& sendJob, bool success);
    void releaseTransportMessage();

    void releaseProtocolSendJob(DoIpStaticPayloadSendJob& sendJob);
    void releaseDiagnosticSendJob(DoIpTransportMessageSendJob& sendJob);

    PayloadPrefixContext _payloadPeekContext;
    IDoIpServerConnection* _connection;
    ::etl::ipool& _diagnosticSendJobPool;
    ::etl::ipool& _protocolSendJobPool;
    ::transport::TransportMessage* _transportMessage;
    DoIpServerTransportConnectionConfig const& _config;
    uint16_t _receiveMessagePayloadLength;
    DoIpConstants::ProtocolVersion _protocolVersion;
    bool _isRoutingActive;
    uint8_t _readBuffer[4];
};

} // namespace doip
