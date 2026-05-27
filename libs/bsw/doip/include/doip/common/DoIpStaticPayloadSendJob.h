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

#include "doip/common/DoIpSimplePayloadSendJob.h"

#include <etl/error_handler.h>
#include <etl/memory.h>

namespace doip
{
/**
 * Class for a send job that consists of a DoIP header and a single already allocated payload
 * buffer.
 */
class DoIpStaticPayloadSendJob : public DoIpSimplePayloadSendJob<DoIpStaticPayloadSendJob>
{
public:
    /**
     * Constructor.
     * \param protocolVersion DoIP protocol version to put into the header
     * \param payloadType type of the payload
     * \param payload The payload buffer that contributes the payload length and the payload data.
     * \param releaseCallback function that will be called to release the job
     */
    DoIpStaticPayloadSendJob(
        uint8_t protocolVersion,
        uint16_t payloadType,
        ::etl::span<uint8_t const> payload,
        ReleaseCallbackType releaseCallback);

    ~DoIpStaticPayloadSendJob() override {}

protected:
    ::etl::span<uint8_t const> getPayloadBuffer(::etl::span<uint8_t> staticBuffer) const override;

private:
    ::etl::span<uint8_t const> _payload;
};

namespace declare
{
/**
 * Class that can be used to represent a DoIP send job with preallocated payload buffer.
 * \tparam BufferSize size of the allocated buffer i.e. the maximum size of a payload that can be
 * stored within this send job.
 */
template<size_t BufferSize>
class DoIpStaticPayloadSendJob : public ::doip::DoIpStaticPayloadSendJob
{
public:
    /**
     * Constructor. This will initialize only the payload length. The payload can then be written
     * by writing the buffer returned by accessPayloadBuffer()
     * \param protocolVersion DoIP protocol version to put into the header
     * \param payloadType type of the payload
     * \param payloadLength length of the payload
     * \param releaseCallback function that will be called to release the job
     */
    DoIpStaticPayloadSendJob(
        uint8_t protocolVersion,
        uint16_t payloadType,
        size_t payloadLength,
        ReleaseCallbackType releaseCallback);
    /**
     * Constructor with temporary payload to copy into the payload buffer.
     * by writing the buffer returned by accessPayloadBuffer()
     * \param protocolVersion DoIP protocol version to put into the header
     * \param payloadType type of the payload
     * \param payload reference to the buffer holding the payload to copy into the allocated buffer
     * \param releaseCallback function that will be called to release the job
     */
    DoIpStaticPayloadSendJob(
        uint8_t protocolVersion,
        uint16_t payloadType,
        ::etl::span<uint8_t const> payload,
        ReleaseCallbackType releaseCallback);

    /**
     * Get write access to the payload buffer allocated with this send job.
     * \return the complete allocated buffer.
     */
    ::etl::span<uint8_t> accessPayloadBuffer();

private:
    uint8_t _payloadBuffer[BufferSize];
};

} // namespace declare

/**
 * Inline implementations.
 */
namespace declare
{
template<size_t BufferSize>
DoIpStaticPayloadSendJob<BufferSize>::DoIpStaticPayloadSendJob(
    uint8_t const protocolVersion,
    uint16_t const payloadType,
    size_t const payloadLength,
    ReleaseCallbackType const releaseCallback)
: ::doip::DoIpStaticPayloadSendJob(
    protocolVersion,
    payloadType,
    ::etl::span<uint8_t const>(_payloadBuffer).subspan(0U, payloadLength),
    releaseCallback)
, _payloadBuffer{}
{}

template<size_t BufferSize>
DoIpStaticPayloadSendJob<BufferSize>::DoIpStaticPayloadSendJob(
    uint8_t const protocolVersion,
    uint16_t const payloadType,
    ::etl::span<uint8_t const> const payload,
    ReleaseCallbackType const releaseCallback)
: ::doip::declare::DoIpStaticPayloadSendJob<BufferSize>(
    protocolVersion, payloadType, payload.size(), releaseCallback)
{
    // Violation would cause out-of-bounds write into _payloadBuffer, corrupting memory.
    ETL_ASSERT(
        payload.size() <= BufferSize, ETL_ERROR_GENERIC("buffer not big enough to hold payload"));
    ::etl::mem_copy(payload.begin(), payload.end(), accessPayloadBuffer().begin());
}

template<size_t BufferSize>
::etl::span<uint8_t> DoIpStaticPayloadSendJob<BufferSize>::accessPayloadBuffer()
{
    return ::etl::span<uint8_t>(_payloadBuffer);
}

} // namespace declare

} // namespace doip
