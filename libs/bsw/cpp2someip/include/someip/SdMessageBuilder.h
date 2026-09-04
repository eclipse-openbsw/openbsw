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

#include <ip/IPEndpoint.h>

#include "someip/SomeIpConstants.h"
#include <etl/span.h>
#include <etl/vector.h>

namespace someip
{
enum class SdMessageReturnCode : uint8_t
{
    SD_MESSAGE_OK,
    SD_MESSAGE_BUFFER_TOO_SMALL, // provided buffer is too small to fit even a shortest message
    SD_MESSAGE_NOT_ENOUGH_SPACE, // not enough free space left
    SD_MESSAGE_INVALID_ADDRESS,  // IP address is unspecified / broken
    SD_MESSAGE_IS_FULL           // no more space for another entry / option
};

namespace internal
{
struct Message final
{
    Message()  = default;
    ~Message() = default;

    ::etl::span<uint8_t> data;
    size_t divider = 0U; // ...a delimiter between entries section and options section
};
} // namespace internal

class SdMessageBuilder final
{
public:
    SdMessageBuilder()  = default;
    ~SdMessageBuilder() = default;

    SdMessageReturnCode startMessage(::etl::span<uint8_t> const& buffer);

    SdMessageReturnCode addFind(
        service_id::type,
        instance_id::type,
        major_version::type,
        minor_version::type,
        ttl::type ttl);

    SdMessageReturnCode addOffer(
        service_id::type,
        instance_id::type,
        major_version::type,
        minor_version::type,
        ttl::type,
        ::ip::IPAddress const&,
        port::type,
        proto::type);

    SdMessageReturnCode addDenounce(
        service_id::type,
        instance_id::type,
        major_version::type,
        minor_version::type,
        ttl::type,
        ::ip::IPAddress const&,
        port::type,
        proto::type);

    SdMessageReturnCode addFindEventgroup(
        service_id::type, instance_id::type, eventgroup_id::type, major_version::type, ttl::type);

    SdMessageReturnCode addPublish(
        service_id::type,
        instance_id::type,
        eventgroup_id::type,
        major_version::type,
        ttl::type,
        ::ip::IPAddress const&,
        port::type,
        proto::type);

    SdMessageReturnCode addUnpublish(
        service_id::type,
        instance_id::type,
        eventgroup_id::type,
        major_version::type,
        ttl::type,
        ::ip::IPAddress const&,
        port::type,
        proto::type);

    SdMessageReturnCode addSubscribe(
        service_id::type,
        instance_id::type,
        eventgroup_id::type,
        major_version::type,
        ttl::type,
        ::ip::IPAddress const&,
        port::type,
        proto::type);

    SdMessageReturnCode addUnsubscribe(
        service_id::type,
        instance_id::type,
        eventgroup_id::type,
        major_version::type,
        ttl::type,
        ::ip::IPAddress const&,
        port::type,
        proto::type);

    SdMessageReturnCode addSubscribeAck(
        service_id::type,
        instance_id::type,
        eventgroup_id::type,
        major_version::type,
        minor_version::type,
        ttl::type);

    SdMessageReturnCode addSubscribeNack(
        service_id::type,
        instance_id::type,
        eventgroup_id::type,
        major_version::type,
        minor_version::type,
        ttl::type);

    SdMessageReturnCode addSubscribeAckMulticast(
        service_id::type,
        instance_id::type,
        eventgroup_id::type,
        major_version::type,
        minor_version::type,
        ttl::type,
        ::ip::IPAddress const&,
        port::type,
        proto::type);

    ::etl::span<uint8_t const> finishMessage(uint16_t sessionId, bool reboot);
    void discardMessage();
    bool isEmpty() const;

private:
    internal::Message _message;
};

} // namespace someip
