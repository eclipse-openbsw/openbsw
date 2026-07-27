/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "middleware/core/Future.h"
#include "middleware/core/MessagePayloadBuilder.h"
#include "middleware/core/ProxyBase.h"
#include "middleware/core/types.h"
#include "middleware/rpc/ProxyMethodImpl.h"

#include <etl/array.h>
#include <etl/delegate.h>
#include <etl/expected.h>
#include <etl/functional.h>
#include <etl/type_traits.h>

#include <cstdint>

namespace middleware::rpc
{

/**
 * \brief Compile-time traits describing a proxy method's input/output types and timeout.
 *
 * \tparam Inp       The input (request payload) type sent to the skeleton (void if none)
 * \tparam Out       The output (response payload) type returned by the skeleton (void for
 *                   acknowledgement-only responses)
 * \tparam Timeout   Timeout duration in milliseconds (0 = no timeout)
 */
template<typename Inp, typename Out, uint32_t Timeout = 0U>
struct MethodTraits
{
    using InputType                   = Inp;
    using OutputType                  = Out;
    static constexpr uint32_t TIMEOUT = Timeout;
};

/**
 * \brief Proxy-side representation of a remote method, managing a pool of futures with
 * callbacks for asynchronous request/response communication.
 *
 * \tparam MethodTraits  Compile-time traits (InputType, OutputType, TIMEOUT)
 * \tparam REQUEST_LIMIT Maximum number of concurrent in-flight requests
 */
template<typename MethodTraits, uint8_t REQUEST_LIMIT = 1U>
class ProxyMethod : public ProxyMethodImpl
{
public:
    using Base       = ProxyMethodImpl;
    using InputType  = typename MethodTraits::InputType;
    using OutputType = typename MethodTraits::OutputType;
    using Result     = etl::conditional_t<
        etl::is_same_v<OutputType, void>,
        etl::expected<void, core::Future::State>,
        etl::expected<etl::reference_wrapper<OutputType const>, core::Future::State>>;
    using Callback                    = etl::delegate<void(Result const&)>;
    static constexpr uint32_t TIMEOUT = MethodTraits::TIMEOUT;

    ProxyMethod() : Base(_futuresArray) {}

    /**
     * \brief Cancels a pending request and frees its associated future.
     *
     * \param requestId The unique identifier of the request to cancel
     * \return HRESULT::Ok if found and cancelled, HRESULT::InstanceNotFound otherwise
     */
    core::HRESULT cancelRequest(uint16_t requestId);

    /**
     * \brief Resets all futures to initial state, abandoning any pending requests.
     */
    void freeAll();

    /**
     * \brief Checks all pending requests for timeouts and invokes callbacks for expired requests.
     *
     * Only active when TIMEOUT > 0.
     */
    void updateTimeouts();

    /**
     * \brief Processes a response message and invokes the corresponding user callback.
     *
     * \param msg The middleware message containing the response or error information
     */
    void answerRequest(core::Message const& msg);

    /**
     * \brief Initiates an asynchronous method call with an input payload.
     *
     * Only available when InputType is not void. Allocates a future, serializes the input
     * payload, and sends the request message.
     *
     * \param proxy    The proxy through which the message is sent
     * \param input    The request payload
     * \param methodId The member ID identifying this method on the wire
     * \param cbk      Callback invoked with either the response value or an error state
     * \return The request ID on success, or an HRESULT error
     */
    template<typename T = InputType>
    etl::enable_if_t<!etl::is_same_v<T, void>, etl::expected<uint16_t, core::HRESULT>>
    callMethod(core::ProxyBase const& proxy, T const& input, uint16_t methodId, Callback cbk);

    /**
     * \brief Initiates an asynchronous method call with no input payload.
     *
     * Only available when InputType is void. Sends a header-only message.
     *
     * \param proxy    The proxy through which the message is sent
     * \param methodId The member ID identifying this method on the wire
     * \param cbk      Callback invoked with either the response value or an error state
     * \return The request ID on success, or an HRESULT error
     */
    template<typename T = InputType>
    etl::enable_if_t<etl::is_same_v<T, void>, etl::expected<uint16_t, core::HRESULT>>
    callMethod(core::ProxyBase const& proxy, uint16_t methodId, Callback cbk);

private:
    /**
     * \brief Invokes the user callback with an error state and resets the future.
     *
     * \param future The future whose error state is reported to the callback
     */
    void notifyWithError(core::Future& future);

    /**
     * \brief Invokes the user callback with the response payload and resets the future.
     *
     * \param msg    The response message containing the payload
     * \param future The future associated with the completed request
     */
    void notifyWithResult(core::Message const& msg, core::Future& future);

    /**
     * \brief Resets a future and its associated callback to their initial state.
     *
     * \param future   The future to reset
     * \param callback The callback to clear
     */
    void clearFutureAndCallback(core::Future& future, Callback& callback);

    etl::array<core::Future, REQUEST_LIMIT> _futuresArray{};
    etl::array<Callback, REQUEST_LIMIT> _callbacks{};
};

// ============================================================================
// Template definitions
// ============================================================================

template<typename MT, uint8_t RL>
core::HRESULT ProxyMethod<MT, RL>::cancelRequest(uint16_t const requestId)
{
    return Base::cancelRequest(requestId);
}

template<typename MT, uint8_t RL>
void ProxyMethod<MT, RL>::freeAll()
{
    Base::freeAll();
}

template<typename MT, uint8_t RL>
void ProxyMethod<MT, RL>::updateTimeouts()
{
    if constexpr (TIMEOUT > 0U)
    {
        Base::updateTimeouts(
            TIMEOUT, etl::make_delegate<ProxyMethod, &ProxyMethod::notifyWithError>(*this));
    }
}

template<typename MT, uint8_t RL>
void ProxyMethod<MT, RL>::answerRequest(core::Message const& msg)
{
    core::Future* const future = futureMatchingRequestId(msg);
    if (future != nullptr)
    {
        if (future->state != core::Future::State::Ready)
        {
            notifyWithError(*future);
        }
        else
        {
            notifyWithResult(msg, *future);
        }
    }
}

template<typename MT, uint8_t RL>
template<typename T>
etl::enable_if_t<!etl::is_same_v<T, void>, etl::expected<uint16_t, core::HRESULT>>
ProxyMethod<MT, RL>::callMethod(
    core::ProxyBase const& proxy, T const& input, uint16_t const methodId, Callback const cbk)
{
    if (!proxy.isInitialized())
    {
        return etl::unexpected<core::HRESULT>(etl::in_place, core::HRESULT::NotRegistered);
    }

    core::Future* future = Base::obtainFuture();
    if (future == nullptr)
    {
        return etl::unexpected<core::HRESULT>(etl::in_place, core::HRESULT::RequestPoolDepleted);
    }

    core::Message msg = proxy.generateMessageHeader(methodId, future->requestId);
    auto const index  = etl::distance(_futuresArray.begin(), future);
    _callbacks[index] = cbk;
    core::HRESULT ret = core::MessagePayloadBuilder::getInstance().allocate(input, msg);
    return Base::callMethod(proxy, msg, ret);
}

template<typename MT, uint8_t RL>
template<typename T>
etl::enable_if_t<etl::is_same_v<T, void>, etl::expected<uint16_t, core::HRESULT>>
ProxyMethod<MT, RL>::callMethod(
    core::ProxyBase const& proxy, uint16_t const methodId, Callback const cbk)
{
    if (!proxy.isInitialized())
    {
        return etl::unexpected<core::HRESULT>(etl::in_place, core::HRESULT::NotRegistered);
    }

    core::Future* future = Base::obtainFuture();
    if (future == nullptr)
    {
        return etl::unexpected<core::HRESULT>(etl::in_place, core::HRESULT::RequestPoolDepleted);
    }

    core::Message msg = proxy.generateMessageHeader(methodId, future->requestId);
    auto const index  = etl::distance(_futuresArray.begin(), future);
    _callbacks[index] = cbk;
    return Base::callMethod(proxy, msg, core::HRESULT::Ok);
}

template<typename MT, uint8_t RL>
void ProxyMethod<MT, RL>::notifyWithError(core::Future& future)
{
    auto const index = etl::distance(_futuresArray.begin(), &future);
    _callbacks[index].call_if(
        etl::unexpected<core::Future::State>(etl::in_place_t{}, future.state));
    clearFutureAndCallback(future, _callbacks[index]);
}

template<typename MT, uint8_t RL>
void ProxyMethod<MT, RL>::notifyWithResult(core::Message const& msg, core::Future& future)
{
    auto const index = etl::distance(_futuresArray.begin(), &future);
    if constexpr (etl::is_same_v<OutputType, void>)
    {
        _callbacks[index].call_if(Result());
    }
    else
    {
        _callbacks[index].call_if(Result(
            etl::in_place_t{},
            etl::cref(core::MessagePayloadBuilder::getInstance().readPayload<OutputType>(msg))));
    }
    clearFutureAndCallback(future, _callbacks[index]);
}

template<typename MT, uint8_t RL>
void ProxyMethod<MT, RL>::clearFutureAndCallback(core::Future& future, Callback& callback)
{
    future.callerTimestamp = 0U;
    future.requestId       = core::INVALID_REQUEST_ID;
    future.state           = core::Future::State::Invalid;
    callback               = {};
}

} // namespace middleware::rpc
