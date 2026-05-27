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

#include <doip/common/DoIpConstants.h>
#include <ip/IPEndpoint.h>
#include <transport/TransportMessage.h>

#include <etl/optional.h>

namespace doip
{
/**
 * Filter interface for DoIP connections.
 */
class IDoIpServerConnectionFilter
{
public:
    /**
     * enumeration defining actions to perform on routing activation request.
     */
    enum class Action : uint8_t
    {
        //  reject the activation and close after having sent the response
        REJECT,
        //  send the activation response code and keep the connection
        KEEP,
        //  continue the activation routing process
        CONTINUE
    };

    /**
     * result of routing activation check.
     */
    class RoutingActivationCheckResult
    {
    public:
        /**
         * Default Constructor:
         * Action: Action::CONTINUE
         * ResponseCode: ROUTING_SUCCESS
         * Internal SourceId: INVALID_ADDRESS
         */
        RoutingActivationCheckResult();

        /**
         * Set the desired action.
         * \return reference to this object
         */
        RoutingActivationCheckResult& setAction(Action action);

        /**
         * Set the desired response code.
         * \return reference to this object
         */
        RoutingActivationCheckResult& setResponseCode(uint8_t responseCode);

        /**
         * Set the optional internal source id.
         * \return reference to this object
         */
        RoutingActivationCheckResult& setInternalSourceAddress(uint16_t internalSourceAddress);

        /**
         * Get action to perform on routing activation.
         * \return action
         */
        Action getAction() const;

        /**
         * Get the response code to send with the activation response.
         * \return response code
         */
        uint8_t getResponseCode() const;

        /**
         * Get the internal source address.
         * \return internal source address
         */
        uint16_t getInternalSourceAddress() const;

        /**
         * Resolve the internal source address.
         * \param defaultSourceAddress source address to return if no internal address is defined
         * \return internal source address if != INVALID_ADDRESS, defaultSourceAddress otherwise
         */
        uint16_t resolveInternalSourceAddress(uint16_t defaultSourceAddress) const;

    private:
        uint16_t _internalSourceAddress;
        Action _action;
        uint8_t _responseCode;
    };

    /**
     * Check a routing activation and specify what to do with it.
     * \param sourceAddress source address of the external tester
     * \param activationType activation type
     * \param socketGroupId socket group identifier
     * \param localEndpoint reference to local endpoint of the connection
     * \param remoteEndpoint reference to remote endpoint of the connection
     * \param isResuming flag indicating whether the connection is resuming. In case of
     *        resuming this method will be called mainly to get the correct priority of
     *        the connection, both action and response code are not obeyed
     * \return result of routing activation containing what to do (action) and
     *         what to return (response code)
     */
    virtual RoutingActivationCheckResult checkRoutingActivation(
        uint16_t sourceAddress,
        uint8_t activationType,
        uint8_t socketGroupId,
        ::ip::IPEndpoint const& localEndpoint,
        ::ip::IPEndpoint const& remoteEndpoint,
        ::etl::optional<uint32_t> const oemField,
        bool isResuming)
        = 0;

protected:
    ~IDoIpServerConnectionFilter()                                             = default;
    IDoIpServerConnectionFilter& operator=(IDoIpServerConnectionFilter const&) = delete;
};

/**
 * Inline implementations.
 */
inline IDoIpServerConnectionFilter::RoutingActivationCheckResult::RoutingActivationCheckResult()
: _internalSourceAddress(::transport::TransportMessage::INVALID_ADDRESS)
, _action(Action::CONTINUE)
, _responseCode(DoIpConstants::RoutingResponseCodes::ROUTING_SUCCESS)
{}

inline IDoIpServerConnectionFilter::RoutingActivationCheckResult&
IDoIpServerConnectionFilter::RoutingActivationCheckResult::setAction(Action const action)
{
    _action = action;
    return *this;
}

inline IDoIpServerConnectionFilter::RoutingActivationCheckResult&
IDoIpServerConnectionFilter::RoutingActivationCheckResult::setResponseCode(
    uint8_t const responseCode)
{
    _responseCode = responseCode;
    return *this;
}

inline IDoIpServerConnectionFilter::RoutingActivationCheckResult&
IDoIpServerConnectionFilter::RoutingActivationCheckResult::setInternalSourceAddress(
    uint16_t const internalSourceAddress)
{
    _internalSourceAddress = internalSourceAddress;
    return *this;
}

inline IDoIpServerConnectionFilter::Action
IDoIpServerConnectionFilter::RoutingActivationCheckResult::getAction() const
{
    return _action;
}

inline uint8_t IDoIpServerConnectionFilter::RoutingActivationCheckResult::getResponseCode() const
{
    return _responseCode;
}

inline uint16_t
IDoIpServerConnectionFilter::RoutingActivationCheckResult::getInternalSourceAddress() const
{
    return _internalSourceAddress;
}

inline uint16_t
IDoIpServerConnectionFilter::RoutingActivationCheckResult::resolveInternalSourceAddress(
    uint16_t const defaultSourceAddress) const
{
    return (_internalSourceAddress != ::transport::TransportMessage::INVALID_ADDRESS)
               ? _internalSourceAddress
               : defaultSourceAddress;
}

} // namespace doip
