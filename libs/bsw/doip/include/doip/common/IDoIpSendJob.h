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

#include <ip/IPEndpoint.h>

#include <etl/intrusive_links.h>
#include <etl/span.h>

namespace doip
{
/**
 * Interface for DoIP send jobs. A send job represents a single DoIP message
 * to be sent. There is no assumption about the content of a DoIP message. A send job
 * consists of a number of buffers that will be sent out one after another.
 */
class IDoIpSendJob : public ::etl::bidirectional_link<0>
{
public:
    virtual ~IDoIpSendJob() = default;

    /**
     * Get the number of buffers to send out for this send job (including header data).
     * \return number of buffers
     */
    virtual uint8_t getSendBufferCount() const = 0;

    /**
     * Get the total number of bytes the send job consists of.
     * \note this is the total length of the send job including the header
     */
    virtual uint16_t getTotalLength() const = 0;

    /**
     * Returns the optional destination endpoint of this send job.
     * \return pointer to endpoint address of the destination if defined
     */
    virtual ::ip::IPEndpoint const* getDestinationEndpoint() const = 0;

    /**
     * Called to release the send job. This will typically call the release callback.
     * \param success true if the job has been sent successfully
     */
    virtual void release(bool success) = 0;

    /**
     * Called to get the n-th of the consecutive send buffers.
     * \param staticBuffer a static buffer (size >= 8) that can be used to put the data into
     * \param index index (< getSendBufferCount()) of the buffer. The buffer with index 0
     *        will typically start with the valid 8 header bytes.
     * \return buffer containing the data. Empty buffers are allowed.
     */
    virtual ::etl::span<uint8_t const>
    getSendBuffer(::etl::span<uint8_t> staticBuffer, uint8_t index) = 0;
};

} // namespace doip
