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
#include "doip/common/DoIpSendJobHelper.h"
#include "doip/common/IDoIpSendJob.h"

#include <etl/delegate.h>
#include <etl/type_traits.h>

namespace doip
{
/**
 * Abstract class for a send job that consists of exactly 2 buffers:
 * a DoIP header (index = 0) and a single payload buffer (index = 1). For returning the payload
 * buffer a function has to implemented.
 */
template<class T>
class DoIpSimplePayloadSendJob : public IDoIpSendJob
{
public:
    using ReleaseCallbackType = ::etl::delegate<void(T& sendJob, bool success)>;

    ~DoIpSimplePayloadSendJob() override {}

    /**
     * Attaches an optional destination endpoint (needed for UDP datagrams) to this send job.
     * \param destinationEndpoint pointer to destination endpoint attached to this job
     */
    void setDestinationEndpoint(::ip::IPEndpoint const* destinationEndpoint);

    /**
     * Sets DoIP message payload type
     * \param payloadType payload type of the DoIP message
     */
    void setPayloadType(uint16_t payloadType);

    uint8_t getSendBufferCount() const override;
    uint16_t getTotalLength() const override;
    ::ip::IPEndpoint const* getDestinationEndpoint() const override;
    void release(bool success) override;
    ::etl::span<uint8_t const>
    getSendBuffer(::etl::span<uint8_t> staticBuffer, uint8_t index) override;

protected:
    /**
     * Constructor (protected for usage in the CRTP).
     * \param protocolVersion DoIP protocol version to put into the header
     * \param payloadType payload type of the DoIP message
     * \param payloadLength length of the payload. This should be the size of the buffer that is
     *        returned from getPayloadBuffer()
     * \param releaseCallback function that will be called to release the send job
     */
    DoIpSimplePayloadSendJob(
        uint8_t protocolVersion,
        uint16_t payloadType,
        uint16_t payloadLength,
        ReleaseCallbackType releaseCallback);

    /**
     * This method returns the payload buffer.
     * \param staticBuffer buffer (>= 8 bytes) that can be used to copy the payload into
     *        The maximum size depends on the configuration of the connection that sends out
     * \return Buffer holding the payload
     */
    virtual ::etl::span<uint8_t const> getPayloadBuffer(::etl::span<uint8_t> staticBuffer) const
        = 0;

private:
    enum class BufferIndex : uint8_t
    {
        HEADER,
        PAYLOAD,
        COUNT,
    };

    ReleaseCallbackType _releaseCallback;
    ::ip::IPEndpoint const* _destinationEndpoint;
    uint16_t _payloadLength;
    uint16_t _payloadType;
    uint8_t _protocolVersion;
};

template<class T>
DoIpSimplePayloadSendJob<T>::DoIpSimplePayloadSendJob(
    uint8_t const protocolVersion,
    uint16_t const payloadType,
    uint16_t const payloadLength,
    ReleaseCallbackType const releaseCallback)
: _releaseCallback(releaseCallback)
, _destinationEndpoint(nullptr)
, _payloadLength(payloadLength)
, _payloadType(payloadType)
, _protocolVersion(protocolVersion)
{
    static_assert(
        etl::is_base_of<DoIpSimplePayloadSendJob<T>, T>::value,
        "Make sure CRTP is used correctly. T must derive from DoIpSimplePayloadSendJob<T>.");
}

template<class T>
uint8_t DoIpSimplePayloadSendJob<T>::getSendBufferCount() const
{
    return static_cast<uint8_t>(BufferIndex::COUNT);
}

template<class T>
uint16_t DoIpSimplePayloadSendJob<T>::getTotalLength() const
{
    return _payloadLength + static_cast<uint16_t>(DoIpConstants::DOIP_HEADER_LENGTH);
}

template<class T>
::ip::IPEndpoint const* DoIpSimplePayloadSendJob<T>::getDestinationEndpoint() const
{
    return _destinationEndpoint;
}

template<class T>
void DoIpSimplePayloadSendJob<T>::release(bool const success)
{
    // No is_valid() check is done here. This will assert if the user doesn't pass a valid one.
    _releaseCallback(static_cast<T&>(*this), success);
}

template<class T>
::etl::span<uint8_t const> DoIpSimplePayloadSendJob<T>::getSendBuffer(
    ::etl::span<uint8_t> const staticBuffer, uint8_t const index)
{
    switch (static_cast<BufferIndex>(index))
    {
        case BufferIndex::HEADER:
        {
            return DoIpSendJobHelper::prepareHeaderBuffer(
                staticBuffer,
                _protocolVersion,
                _payloadType,
                static_cast<uint32_t>(_payloadLength));
        }
        case BufferIndex::PAYLOAD:
        {
            return getPayloadBuffer(staticBuffer);
        }
        default:
        {
            return {};
        }
    }
}

/**
 * Inline implementation.
 */
template<class T>
inline void DoIpSimplePayloadSendJob<T>::setDestinationEndpoint(
    ::ip::IPEndpoint const* const destinationEndpoint)
{
    _destinationEndpoint = destinationEndpoint;
}

template<class T>
inline void DoIpSimplePayloadSendJob<T>::setPayloadType(uint16_t const payloadType)
{
    _payloadType = payloadType;
}
} // namespace doip
