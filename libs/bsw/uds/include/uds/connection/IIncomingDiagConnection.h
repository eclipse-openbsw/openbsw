#pragma once

#include "uds/connection/ErrorCode.h"
#include "uds/connection/PositiveResponse.h"
#include "uds/DiagReturnCode.h"

#include <cstdint>

namespace uds
{
class AbstractDiagJob;
class IDiagSessionManager;
class NestedDiagRequest;

/**
 * Interface for incoming diagnostic connections.
 */
class IIncomingDiagConnection
{
public:
    virtual ~IIncomingDiagConnection() = default;

    /**
     * Opens the diagnostic connection.
     * \param activatePending Whether to activate pending responses
     */
    virtual void open(bool activatePending) = 0;

    /**
     * Terminates the diagnostic connection.
     */
    virtual void terminate() = 0;

    /**
     * Adds an identifier to the response buffer.
     */
    virtual void addIdentifier() = 0;

    /**
     * Returns the number of identifiers added to the response.
     */
    virtual uint16_t getNumIdentifiers() const = 0;

    /**
     * Returns an identifier for a given index.
     */
    virtual uint8_t getIdentifier(uint16_t idx) const = 0;

    /**
     * Returns the maximum response length.
     */
    virtual uint16_t getMaximumResponseLength() const = 0;

    /**
     * Releases the request and returns a positive response buffer.
     */
    virtual PositiveResponse& releaseRequestGetResponse() = 0;

    /**
     * Sends a positive response.
     */
    virtual ErrorCode sendPositiveResponse(AbstractDiagJob& sender) = 0;

    /**
     * Sends a positive response with a specific payload length.
     */
    virtual ErrorCode sendPositiveResponseInternal(uint16_t length, AbstractDiagJob& sender) = 0;

    /**
     * Sends a negative response with the specified response code.
     */
    virtual ErrorCode sendNegativeResponse(uint8_t responseCode, AbstractDiagJob& sender) = 0;

    /**
     * Suppresses positive response transmission.
     */
    virtual void suppressPositiveResponse() = 0;

    /**
     * Disables response timeout.
     */
    virtual void disableResponseTimeout() = 0;

    /**
     * Disables global connection timeout.
     */
    virtual void disableGlobalTimeout() = 0;

    /**
     * Checks if the connection is busy sending a response.
     */
    virtual bool isBusy() const = 0;

    /**
     * Starts a nested diagnostic request.
     */
    virtual DiagReturnCode::Type startNestedRequest(
        AbstractDiagJob& sender,
        NestedDiagRequest& nestedRequest,
        uint8_t const request[],
        uint16_t requestLength) = 0;

    /**
     * Terminates the current nested request.
     * \return true if nested request was terminated, false otherwise
     */
    virtual bool terminateNestedRequest() = 0;

    /**
     * Sends the response.
     */
    virtual ErrorCode sendResponse() = 0;

    /**
     * Returns the service ID of the diagnostic request.
     */
    virtual uint8_t getServiceId() const = 0;

    /**
     * Returns the target address of the diagnostic connection.
     */
    virtual uint16_t getTargetAddress() const = 0;

    /**
     * Returns the source address of the diagnostic connection.
     */
    virtual uint16_t getSourceAddress() const = 0;

    /**
     * Sets the diagnostic session manager.
     */
    virtual void setDiagSessionManager(IDiagSessionManager* sessionManager) = 0;

    /**
     * Sets the source address.
     */
    virtual void setSourceAddress(uint16_t address) = 0;

    /**
     * Sets the target address.
     */
    virtual void setTargetAddress(uint16_t address) = 0;

    /**
     * Sets the response source address.
     */
    virtual void setResponseSourceAddress(uint16_t address) = 0;

    /**
     * Sets the service ID.
     */
    virtual void setServiceId(uint8_t id) = 0;
};

} // namespace uds