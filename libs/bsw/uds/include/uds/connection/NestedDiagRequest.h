// Copyright 2024 Accenture.

#pragma once

#include "uds/DiagReturnCode.h"

#include <etl/span.h>

#include <cstdint>

namespace uds
{
class AbstractDiagJob;
class IncomingDiagConnection;

/**
 * Helper class for handling nested diagnostic requests.
 */
class NestedDiagRequest
{
public:
    /**
     * \param prefixLength Length of prefix. This defines the number of leading identifiers
     *        bytes to ignore when a nested request call addIdentifier()
     */
    explicit NestedDiagRequest(uint8_t prefixLength);

    NestedDiagRequest(NestedDiagRequest const&)            = delete;
    NestedDiagRequest& operator=(NestedDiagRequest const&) = delete;

    /**
     * Initialize a new nested request. The request is (eventually converted) and stored.
     * \param sender Reference to abstract diag job that will send the response. This sender
     *        is stored during the nested request in senderJob.
     * \param messageBuffer Available buffer for both response and stored request
     * \param request Incoming request
     */
    void init(
        AbstractDiagJob& sender,
        ::etl::span<uint8_t> const& messageBuffer,
        ::etl::span<uint8_t const> const& request);

    /**
     * Prepare the next nested request if existing
     * \return true if a next nested is existing and has been prepared
     */
    bool prepareNextRequest();

    /**
     * Process nested request
     * \param connection Connection for executing
     * \result Result of processing
     */
    DiagReturnCode::Type processNextRequest(IncomingDiagConnection& connection);

    /**
     * Get buffer for response
     * \return Response buffer
     */
    ::etl::span<uint8_t> getResponseBuffer();

    /**
     * Handle a negative response code just after nested request has been processed
     * \param responseCode Response code of nested request
     * \return Adjusted code, if DiagReturnCode::OK the next request is triggered
     */
    void handleNegativeResponseCode(DiagReturnCode::Type responseCode);

    /**
     * Handle overflow of response message buffer
     */
    void handleResponseOverflow();

    /**
     * Mark identifier as being consumed
     */
    void addIdentifier();
    /**
     * Get identifier
     * \param index Index of identifier to get
     * \return Identifier if index is valid, 0 otherwise
     */
    uint8_t getIdentifier(uint16_t idx) const;

    /**
     * Get maximum length for the next nested response
     * \return Maximum length for response
     */

    uint16_t getMaxNestedResponseLength() const;
    /**
     * Called if a nested request has succeeded. The available message buffer for the next
     * requests will be reduce by the message buffer the was consumed by the last nested response.
     * \param responseLength Length of response
     */
    void setNestedResponseLength(uint16_t responseLength);

protected:
    /**
     * Get needed length of storage for a given request. The default implementation returns the
     * size of the request. Overload this if storing the request consumes more or less bytes.
     * \note While the request is stored within the same message buffer that can be used for the
     *       response it is crucial to think about memory consumption of the stored request
     * \param request Request to store
     */
    virtual uint16_t getStoredRequestLength(::etl::span<uint8_t const> const& request) const;

    /**
     * Store the request. This will be called only if the return value of startRequest() is greater
     * 0. The default implementation stores the complete request. Overload this method if you need
     * to convert the request or if not all of it needs to be stored.
     * \param request The request to store
     * \param dest Reference to buffer that may be used for storing
     */
    virtual void
    storeRequest(::etl::span<uint8_t const> const& request, ::etl::span<uint8_t> dest) const;

    /**
     * Prepare a nested request
     * \param storedRequest Reference to buffer holding
     * \return Buffer holding request if available, empty buffer otherwise
     */
    virtual ::etl::span<uint8_t const>
    prepareNestedRequest(::etl::span<uint8_t const> const& storedRequest) = 0;
    /**
     * Process next nested request
     * \param connection Connection
     * \param request Reference to first request byte
     * \param requestLength Length of request
     * \result Result of processing
     */
    virtual DiagReturnCode::Type processNestedRequest(
        IncomingDiagConnection& connection, uint8_t const request[], uint16_t requestLength)
        = 0;
    /**
     * Adjust response code of nested request
     * \param responseCode Response code of request
     */

    virtual void handleNestedResponseCode(DiagReturnCode::Type responseCode);
    /**
     * Handle overflow of response message buffer
     */

    virtual void handleOverflow();

    /**
     * Consume a given number of bytes from stored request
     * \param consumedLength Number of consumed bytes
     * \return Buffer holding consumed data, only the available bytes are returned
     */
    ::etl::span<uint8_t const> consumeStoredRequest(uint16_t consumedLength);

public:
    uint16_t responseLength() const { return _responseLength; }

    AbstractDiagJob* senderJob;
    AbstractDiagJob* pendingResponseSender;
    uint8_t const prefixLength;
    bool isPendingSent;
    DiagReturnCode::Type responseCode;

protected:
    uint16_t _responseLength = 0;
    ::etl::span<uint8_t> _messageBuffer;
    uint16_t _storedRequestLength;
    uint8_t _numIdentifiers;
    uint8_t _numPrefixIdentifiers;
    ::etl::span<uint8_t const> _nestedRequest = {};
};

/**
 * Inline implementation
 */
} // namespace uds
