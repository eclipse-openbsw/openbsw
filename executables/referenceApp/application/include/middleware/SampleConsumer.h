/********************************************************************************
 * Copyright (c) 2026 KPIT Group
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "app/DemoLogger.h"
#include "ets/EtsProxy.h"

#include <middleware/core/Future.h>
#include <util/logger/Logger.h>

#include <etl/delegate.h>
#include <etl/expected.h>
#include <etl/functional.h>

#include <cstdint>

namespace middlewaresample
{

/**
 * Consumes the peer node's instance of the service through the generated proxy. It calls
 * echoUINT8, gets and sets the TestFieldUINT8 field, and logs the field's
 * notifications and the TestEventUINT8Multicast events it receives.
 */
class SampleConsumer final
{
public:
    void init(
        ::ets::Ets::internal::InstanceId const instanceId,
        ::middleware::core::ClusterId const cluster)
    {
        static_cast<void>(_proxy.init(instanceId, cluster));
        _proxy.testEventUint8Multicast.setReceiveHandler(
            ::etl::delegate<void(uint8_t const&)>::create<SampleConsumer, &SampleConsumer::onEvent>(
                *this));
        _proxy.testFieldUint8.setReceiveHandler(
            ::etl::delegate<void(
                uint8_t const&)>::create<SampleConsumer, &SampleConsumer::onFieldChanged>(*this));
    }

    void deInit() { _proxy.deInit(); }

    /// Sends one request per step, in turn: echoUINT8, the field's getter, its setter.
    void step()
    {
        using Proxy = ::ets::Ets::proxy::EtsProxy;
        using Field = ::ets::Ets::proxy::TestFieldUINT8Attribute;

        switch (_step)
        {
            case 0U:
            {
                ++_value;
                static_cast<void>(_proxy.echoUint8(
                    _value,
                    Proxy::EchoUINT8Callback::create<SampleConsumer, &SampleConsumer::onEcho>(
                        *this)));
                break;
            }
            case 1U:
            {
                static_cast<void>(_proxy.testFieldUint8.get(
                    Field::GetterCallback::create<SampleConsumer, &SampleConsumer::onGet>(*this)));
                break;
            }
            default:
            {
                static_cast<void>(_proxy.testFieldUint8.set(
                    _value,
                    Field::SetterCallback::create<SampleConsumer, &SampleConsumer::onSet>(*this)));
                break;
            }
        }
        _step = static_cast<uint8_t>((_step + 1U) % STEP_COUNT);
    }

private:
    static constexpr uint8_t STEP_COUNT = 3U;

    using Result = ::etl::
        expected<::etl::reference_wrapper<uint8_t const>, ::middleware::core::Future::State>;

    void onEvent(uint8_t const& value)
    {
        ::util::logger::Logger::info(
            ::util::logger::DEMO,
            "[MiddlewareSample] TestEventUINT8Multicast received, value=%u",
            static_cast<unsigned>(value));
    }

    void onFieldChanged(uint8_t const& value)
    {
        ::util::logger::Logger::info(
            ::util::logger::DEMO,
            "[MiddlewareSample] TestFieldUINT8 notified, value=%u",
            static_cast<unsigned>(value));
    }

    void onEcho(Result const& result) { logResult("echoUINT8", result); }

    void onGet(Result const& result) { logResult("TestFieldUINT8 get", result); }

    void onSet(Result const& result) { logResult("TestFieldUINT8 set", result); }

    static void logResult(char const* const call, Result const& result)
    {
        if (result.has_value())
        {
            ::util::logger::Logger::info(
                ::util::logger::DEMO,
                "[MiddlewareSample] %s response, value=%u",
                call,
                static_cast<unsigned>(result.value().get()));
        }
        else
        {
            ::util::logger::Logger::warn(
                ::util::logger::DEMO,
                "[MiddlewareSample] %s failed, state=%u",
                call,
                static_cast<unsigned>(result.error()));
        }
    }

    ::ets::Ets::proxy::EtsProxy _proxy;
    uint8_t _value{0U};
    uint8_t _step{0U};
};

} // namespace middlewaresample
