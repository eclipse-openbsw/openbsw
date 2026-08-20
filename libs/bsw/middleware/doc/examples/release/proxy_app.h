/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <features/communication/dummy_serviceCommon.h>
#include <features/communication/dummy_serviceProxy.h>

#include <etl/delegate.h>
#include <etl/expected.h>
#include <etl/functional.h>
#include <etl/optional.h>

#include <cstdint>

namespace application
{

namespace
{
constexpr uint8_t FOO_VALUE_A            = 30U;
constexpr uint8_t FOO_VALUE_B            = 70U;
constexpr uint8_t FNF_VALUE_A1           = 20U;
constexpr uint8_t FNF_VALUE_A2           = 50U;
constexpr uint8_t FNF_VALUE_B1           = 40U;
constexpr uint8_t FNF_VALUE_B2           = 90U;
constexpr uint32_t ATTRIBUTE_WRITE_VALUE = 0x0000002AU;
} // namespace

// [service-proxy-wrapper-start]
class ProxyApp : public features::communication::DummyService::proxy::DummyServiceProxy
{
public:
    using Base                 = features::communication::DummyService::proxy::DummyServiceProxy;
    using Foo                  = features::communication::DummyService::Foo;
    using Baz                  = features::communication::DummyService::Baz;
    using MwInstanceId         = features::communication::DummyService::internal::InstanceId;
    using FireAndForgetPayload = features::communication::DummyService::FireAndForgetPayload;
    using AttributeType
        = features::communication::DummyService::proxy::SimpleFieldAttribute::AttributeType;
    using AsyncMethodResult
        = features::communication::DummyService::proxy::DummyServiceProxy::AsyncMethodResult;
    using AsyncMethodCallback
        = features::communication::DummyService::proxy::DummyServiceProxy::AsyncMethodCallback;
    using AttributeGetterCallback
        = features::communication::DummyService::proxy::SimpleFieldAttribute::GetterCallback;
    using AttributeGetterResult
        = features::communication::DummyService::proxy::SimpleFieldAttribute::GetterResult;
    using AttributeReceiveCallback = features::communication::DummyService::proxy::
        SimpleFieldAttribute::OnFieldChangedCallback;
    using EventReceiveCallback = features::communication::DummyService::proxy::
        SimpleBroadcastEvent::OnFieldChangedCallback;
    using MwResult = etl::expected<uint16_t, ::middleware::core::HRESULT>;

    void startup()
    {
        if (init())
        {
            setEventReceiveHandler();
            setAttributeReceiveHandler();
        }
    }

    void shutdown() { Base::deInit(); }

    // [service-proxy-method-start]
    void runAsyncMethod(Foo const& input)
    {
        MwResult const result = Base::asyncMethod(
            input, etl::make_delegate<ProxyApp, &ProxyApp::asyncMethodResponse_>(*this));
        currentActiveRequestId_
            = result.has_value() ? etl::optional<uint16_t>{result.value()} : etl::nullopt;
    }

    // [service-proxy-method-end]

    // [service-proxy-fire-and-forget-start]
    MwResult runFireAndForgetMethod(FireAndForgetPayload const& payload)
    {
        return Base::fireAndForgetMethod(payload);
    }

    // [service-proxy-fire-and-forget-end]

    // [service-proxy-attribute-read-start]
    MwResult runAttributeGet()
    {
        return this->simpleField.get(
            etl::make_delegate<ProxyApp, &ProxyApp::attributeGetterResponse_>(*this));
    }

    // [service-proxy-attribute-read-end]

    // [service-proxy-attribute-write-start]
    MwResult runAttributeSet(uint32_t const value) { return this->simpleField.set(value); }

    // [service-proxy-attribute-write-end]

    [[nodiscard]] uint32_t receivedAttributeValue() const { return receivedAttributeValue_; }

    [[nodiscard]] uint32_t receivedEventValue() const { return receivedEventValue_; }

    [[nodiscard]] bool isRequestIdActive() const { return currentActiveRequestId_.has_value(); }

    [[nodiscard]] uint16_t getRequestIdActive() const { return currentActiveRequestId_.value(); }

    [[nodiscard]] bool isResponseValid() const { return latestResult_.has_value(); }

    [[nodiscard]] Baz getResponseValue() const { return latestResult_.value(); }

private:
    bool init()
    {
        return Base::init(MwInstanceId::InstanceId_1, ::middleware::core::ClusterId::Core1)
                   == ::middleware::core::HRESULT::Ok
               || Base::isInitialized();
    }

    // [service-attribute-subscription-start]
    void setAttributeReceiveHandler()
    {
        this->simpleField.setReceiveHandler(
            etl::make_delegate<ProxyApp, &ProxyApp::attributeChanged_>(*this));
    }

    // [service-attribute-subscription-end]

    // [service-proxy-method-subscription-start]
    void setEventReceiveHandler()
    {
        this->simpleBroadcast.setReceiveHandler(
            etl::make_delegate<ProxyApp, &ProxyApp::eventReceived_>(*this));
    }

    // [service-proxy-method-subscription-end]

    void asyncMethodResponse_(AsyncMethodResult const& result)
    {
        latestResult_
            = result.has_value() ? etl::optional<Baz>{result.value().get()} : etl::nullopt;
        currentActiveRequestId_ = etl::nullopt;
    }

    void attributeGetterResponse_(AttributeGetterResult const& result)
    {
        if (result.has_value())
        {
            receivedAttributeValue_ = result.value().get();
        }
    }

    // [service-proxy-attribute-subscription-callback-start]
    void attributeChanged_(uint32_t const& value) { receivedAttributeValue_ = value; }

    // [service-proxy-attribute-subscription-callback-end]

    // [service-proxy-broadcast-callback-start]
    void eventReceived_(uint32_t const& value) { receivedEventValue_ = value; }

    // [service-proxy-broadcast-callback-end]

    etl::optional<uint16_t> currentActiveRequestId_;
    etl::optional<Baz> latestResult_;
    uint32_t receivedAttributeValue_{};
    uint32_t receivedEventValue_{};
};

// [service-proxy-wrapper-end]

} // namespace application
