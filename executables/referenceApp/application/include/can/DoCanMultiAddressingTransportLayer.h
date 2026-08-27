/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <docan/addressing/DoCanNormalAddressing.h>
#include <docan/transport/DoCanTransportLayer.h>
#include <etl/span.h>
#include <transport/AbstractTransportLayer.h>
#include <transport/ITransportMessageProvidingListener.h>

namespace transport
{
class TransportMessage;
class ITransportMessageProcessedListener;
} // namespace transport

namespace can
{
/**
 * Composite transport layer that showcases how a single application can support several ISO
 * 15765-2 addressing schemes for the same bus (CAN_0) at the same time.
 *
 * ITransportSystem (and, transitively, TransportRouterSimple) only allows a single
 * ::transport::AbstractTransportLayer to be registered per bus id, so this class acts as that
 * single registered layer and internally dispatches to one of four inner
 * ::docan::DoCanTransportLayer instances - one per addressing scheme - based on the tester
 * address:
 * - Normal Addressing:                tester address 0xF1
 * - Extended Addressing:              tester address 0xF4
 * - Range Extended Addressing:        tester address 0xF2
 * - Normal Fixed Addressing:          tester address 0xF3
 *
 * Dispatch on the send() (outbound, ECU-to-tester) path is done by looking at the target address
 * of the ::transport::TransportMessage to send, since for an outbound message the target address
 * is exactly the tester's address, which is scheme-specific by construction. Dispatch on the
 * receive path needs no special handling here: each inner DoCanTransportLayer only reacts to the
 * CAN identifiers configured for its own addressing scheme, so incoming requests are routed to
 * the right instance for free at the CAN filter level.
 *
 * Since only this composite is registered with the router, only its own
 * fProvidingListenerHelper gets wired up (by ITransportSystem::addTransportLayer()) to point at
 * the actual upstream ITransportMessageProvider/ITransportMessageListener (e.g. the transport
 * router). propagateProvidingListener() must be called once, after this instance has been
 * registered, to forward those same pointers into each inner DoCanTransportLayer so that each of
 * them can independently allocate buffers for, and hand up, received messages exactly as if it
 * had been registered directly.
 */
class DoCanMultiAddressingTransportLayer final : public ::transport::AbstractTransportLayer
{
public:
    using DataLinkLayerType  = ::docan::DoCanNormalAddressing<>::DataLinkLayerType;
    using TransportLayerType = ::docan::DoCanTransportLayer<DataLinkLayerType>;

    explicit DoCanMultiAddressingTransportLayer(uint8_t busId);

    /**
     * Binds the inner transport layers used for dispatching, one per addressing scheme. Must be
     * called once, before init()/send() are used, after the individual layers have been created.
     */
    void bind(
        TransportLayerType& normalAddressingLayer,
        TransportLayerType& extendedAddressingLayer,
        TransportLayerType& rangeExtendedAddressingLayer,
        TransportLayerType& normalFixedAddressingLayer);

    /**
     * Forwards this instance's providing listener (provider/listener pointers set up by
     * ITransportSystem::addTransportLayer()) to all four inner transport layers. Must be called
     * once, after this instance has been registered via ITransportSystem::addTransportLayer().
     */
    void propagateProvidingListener();

    ErrorCode send(
        ::transport::TransportMessage& transportMessage,
        ::transport::ITransportMessageProcessedListener* pNotificationListener) override;

private:
    /**
     * Forwards incoming messages/buffer requests for the Normal Fixed Addressing scheme
     * unchanged, except that a functional (broadcast) request's target address is remapped from
     * the legislated wire-level group address (NORMAL_FIXED_ADDRESSING_FUNCTIONAL_ADDRESS, 0x33)
     * to the single, scheme-independent functional address
     * (::transport::TransportConfiguration::FUNCTIONAL_ALL_ISO14229, 0xDF) expected by the
     * shared upstream transport router/UDS dispatcher. ::docan::DoCanNormalFixedAddressingFilter
     * itself deliberately reports the raw wire address unmodified (see its documentation), so
     * this remapping has to happen here instead, once per received message - it does not apply
     * to the other three (table-based) addressing schemes, whose functional target address is
     * already configured (in DoCanSystem) to equal FUNCTIONAL_ALL_ISO14229 on the wire.
     */
    class NormalFixedFunctionalAddressRemapper
    : public ::transport::ITransportMessageProvidingListener
    {
    public:
        NormalFixedFunctionalAddressRemapper();

        /// Binds the real upstream provider/listener to forward to (after remapping).
        void bind(
            ::transport::ITransportMessageProvider& provider,
            ::transport::ITransportMessageListener& listener);

        ErrorCode getTransportMessage(
            uint8_t srcBusId,
            uint16_t sourceAddress,
            uint16_t targetAddress,
            uint16_t size,
            ::etl::span<uint8_t const> const& peek,
            ::transport::TransportMessage*& pTransportMessage) override;

        void releaseTransportMessage(::transport::TransportMessage& transportMessage) override;

        void dump() override;

        ReceiveResult messageReceived(
            uint8_t sourceBusId,
            ::transport::TransportMessage& transportMessage,
            ::transport::ITransportMessageProcessedListener* pNotificationListener) override;

    private:
        static uint16_t remapTargetAddress(uint16_t targetAddress);

        ::transport::ITransportMessageProvider* _provider;
        ::transport::ITransportMessageListener* _listener;
    };

    TransportLayerType* getTransportLayerForTesterId(uint16_t testerId) const;

    TransportLayerType* _normalAddressingLayer;
    TransportLayerType* _extendedAddressingLayer;
    TransportLayerType* _rangeExtendedAddressingLayer;
    TransportLayerType* _normalFixedAddressingLayer;
    NormalFixedFunctionalAddressRemapper _normalFixedFunctionalAddressRemapper;
};

} // namespace can
