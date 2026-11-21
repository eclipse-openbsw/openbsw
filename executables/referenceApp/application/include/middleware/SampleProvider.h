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
#include "ets/EtsSkeleton.h"

#include <util/logger/Logger.h>

#include <cstdint>

namespace middlewaresample
{

/**
 * Provides this node's instance of the service through the generated skeleton.
 * It answers echoUINT8, serves the TestFieldUINT8 field and publishes
 * TestEventUINT8Multicast.
 *
 * SampleSomeIp binds only these members on SOME/IP. The stack rejects a request
 * for any other method of the service as unknown, so the empty handlers below
 * are never called.
 */
class SampleProvider final : public ::ets::Ets::skeleton::EtsSkeleton
{
public:
    using ResponseInfo = ::middleware::core::ResponseBufferBase::SkeletonResponseInfo;

    /// Publishes the next counter value as TestEventUINT8Multicast.
    void publish()
    {
        ++_counter;
        static_cast<void>(testEventUint8Multicast.send(_counter));
    }

    void echoUint8(uint8_t const& value, ResponseInfo& response) override
    {
        ::util::logger::Logger::info(
            ::util::logger::DEMO,
            "[MiddlewareSample] echoUINT8 request, value=%u",
            static_cast<unsigned>(value));
        static_cast<void>(respondEchoUINT8(response, value));
    }

    void setTestFieldUINT8Attribute(uint8_t const& value, ResponseInfo& response) override
    {
        // Store the new value, broadcast it to subscribers, then respond with the value now held.
        testFieldUint8.set(value);
        static_cast<void>(testFieldUint8.send());
        static_cast<void>(respondSetTestFieldUINT8Attribute(response, testFieldUint8.get()));
        ::util::logger::Logger::info(
            ::util::logger::DEMO,
            "[MiddlewareSample] TestFieldUINT8 set, value=%u",
            static_cast<unsigned>(value));
    }

    // Not bound on SOME/IP.
    void clientServiceActivate(uint8_t const&) override {}

    void clientServiceDeactivate(uint8_t const&) override {}

    void clientServiceSubscribeEventgroup(
        ::ets::Ets::ClientServiceSubscribeEventgroupRequest const&) override
    {}

    void resetInterface() override {}

    void suspendInterface(::ets::Ets::ClientServiceSubscribeEventgroupRequest const&) override {}

    void triggerEventUint8(::ets::Ets::TriggerEventUINT8Request const&) override {}

    void triggerEventUint8Array(::ets::Ets::TriggerEventUINT8Request const&) override {}

    void triggerEventUint8E2E(::ets::Ets::TriggerEventUINT8Request const&) override {}

    void triggerEventUint8Reliable(::ets::Ets::TriggerEventUINT8Request const&) override {}

    void triggerEventUint8Multicast(::ets::Ets::TriggerEventUINT8Request const&) override {}

    void checkByteOrder(::ets::Ets::CheckByteOrderRequest const&, ResponseInfo&) override {}

    void echoCommonDatatypes(::ets::Ets::EchoCommonDatatypesRequest const&, ResponseInfo&) override
    {}

    void echoEnum(uint8_t const&, ResponseInfo&) override {}

    void echoFloat64(double const&, ResponseInfo&) override {}

    void echoInt64(int64_t const&, ResponseInfo&) override {}

    void echoInt8(int8_t const&, ResponseInfo&) override {}

    void echoStaticUint8Array(::ets::Ets::EtsUint8ArrayFixed5 const&, ResponseInfo&) override {}

    void echoUint8Array(::ets::Ets::EtsUint8ArrayDyn32 const&, ResponseInfo&) override {}

    void echoUint8Array8BitLength(::ets::Ets::EtsUint8ArrayDyn8 const&, ResponseInfo&) override {}

    void echoUint8Array16BitLength(::ets::Ets::EtsUint8ArrayDyn16 const&, ResponseInfo&) override {}

    void echoUint8Array2Dim(::ets::Ets::EtsUint8ArrayDyn32Dyn32 const&, ResponseInfo&) override {}

    void
    echoUint8ArrayMinSize(::ets::Ets::EtsUint8ArrayDyn32Min3Max5 const&, ResponseInfo&) override
    {}

    void echoUint8E2E(::ets::Ets::EchoUINT8E2ERequest const&, ResponseInfo&) override {}

    void echoUint8Reliable(uint8_t const&, ResponseInfo&) override {}

    void echoUnion(::ets::Ets::EtsUnion12 const&, ResponseInfo&) override {}

    void echoUtf16Dynamic(::ets::Ets::EtsUtf16beDynamic const&, ResponseInfo&) override {}

    void echoUtf16Fixed(::ets::Ets::EtsUtf16beFixed64 const&, ResponseInfo&) override {}

    void echoUtf8Dynamic(::ets::Ets::EtsUtf8Dynamic const&, ResponseInfo&) override {}

    void echoUtf8Fixed(::ets::Ets::EtsUtf8Fixed64 const&, ResponseInfo&) override {}

    void clientServiceGetLastValueOfEventTcp(ResponseInfo&) override {}

    void clientServiceGetLastValueOfEventUdpUnicast(ResponseInfo&) override {}

    void clientServiceGetLastValueOfEventUdpMulticast(ResponseInfo&) override {}

    void echoBitfields(::ets::Ets::EchoBitfieldsRequest const&, ResponseInfo&) override {}

private:
    uint8_t _counter{0U};
};

} // namespace middlewaresample
