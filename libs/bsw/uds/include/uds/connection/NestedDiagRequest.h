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
     * Get prefix length
     * \return Length of prefix
     */
    uint8_t getPrefixLength() const;

    /**
     * Initialize a new nested request. The request is (eventually converted) and stored.
     * \param sender Reference to abstract diag job that will send the response. This sender
     *        is stored during the nested request and can be requested with getSender().
     * \param messageBuffer Available buffer for both response and stored request
     * \param request Incoming request
     */
    void init(
        AbstractDiagJob& sender,
        ::etl::span<uint8_t> const& messageBuffer,
        ::etl::span<uint8_t const> const& request);

    /**
     * Get the sender for the response.
     * \return Sender of the response
     */
    AbstractDiagJob* getSender() const;

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
     * Get next nested request
     * \return Buffer holding the next nested request
     */
    ::etl::span<uint8_t const> getNextRequest() const;

    /**
     * Get buffer for response
     * \return Response buffer
     */
    ::etl::span<uint8_t> getResponseBuffer();

    /**
     * Get length of response
     * \return Length of response
     */
    uint16_t getResponseLength() const;

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
     * Get response code of complete request
     * \return Response code
     */
    DiagReturnCode::Type getResponseCode() const;

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

    /**
     * Set sender of pending response
     * \param pendingResponseSender Sender of pending response
     */
    void setPendingResponseSender(AbstractDiagJob* pendingResponseSender);

    /**
     * Get sender of pending response
     * \return Sender of pending response
     */
    AbstractDiagJob* getPendingResponseSender() const;

    /**
     * Set pending response has been sent at least once
     */
    void setIsPendingSent();

    /**
     * Check whether a pending response has already been sent
     * \return true if pending response has been sent
     */
    bool isPendingSent() const;

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

    /**
     * Set the response code for the complete request
     */
    void setResponseCode(DiagReturnCode::Type responseCode);

private:
    AbstractDiagJob* fSender;
    AbstractDiagJob* fPendingResponseSender;
    ::etl::span<uint8_t> fMessageBuffer;
    ::etl::span<uint8_t const> fNestedRequest;
    uint16_t fStoredRequestLength;
    uint16_t fResponseLength;
    uint8_t fPrefixLength;
    uint8_t fNumIdentifiers;
    uint8_t fNumPrefixIdentifiers;
    bool fIsPendingSent;
    DiagReturnCode::Type fResponseCode;
};

/**
 * Inline implementation
 */
inline uint8_t NestedDiagRequest::getPrefixLength() const { return fPrefixLength; }

inline AbstractDiagJob* NestedDiagRequest::getSender() const { return fSender; }

inline ::etl::span<uint8_t const> NestedDiagRequest::getNextRequest() const
{
    return fNestedRequest;
}

inline uint16_t NestedDiagRequest::getResponseLength() const { return fResponseLength; }

inline void
NestedDiagRequest::setPendingResponseSender(AbstractDiagJob* const pendingResponseSender)
{
    fPendingResponseSender = pendingResponseSender;
}

inline AbstractDiagJob* NestedDiagRequest::getPendingResponseSender() const
{
    return fPendingResponseSender;
}

inline void NestedDiagRequest::setIsPendingSent() { fIsPendingSent = true; }

inline bool NestedDiagRequest::isPendingSent() const { return fIsPendingSent; }

inline DiagReturnCode::Type NestedDiagRequest::getResponseCode() const { return fResponseCode; }

inline void NestedDiagRequest::setResponseCode(DiagReturnCode::Type const responseCode)
{
    fResponseCode = responseCode;
}

} // namespace uds
