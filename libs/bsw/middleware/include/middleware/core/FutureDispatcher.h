// Copyright 2025 BMW AG

#pragma once

#include "middleware/core/Future.h"
#include "middleware/core/FutureDispatcherBase.h"
#include "middleware/core/MessagePayloadBuilder.h"
#include "middleware/core/types.h"
#include "middleware/time/SystemTimerProvider.h"

#include <etl/array.h>
#include <etl/delegate.h>
#include <etl/expected.h>
#include <etl/tuple.h>
#include <etl/vector.h>

#include <cstdint>

namespace middleware
{
namespace core
{

/**
 * Compile-time traits describing a dispatcher's argument type, member ID, copy policy,
 * and optional timeout.
 *
 * \tparam Arg The argument/return type of the dispatched method
 * \tparam MemberIdValue The member ID associated with this dispatcher
 * \tparam CopyOnSend Whether payloads are copied on send
 * \tparam TIMEOUT Timeout duration in milliseconds (0 = no timeout)
 */
template<typename Arg, uint16_t MemberIdValue, bool CopyOnSend, uint32_t TIMEOUT = 0U>
struct DispatcherTraits
{
    using ArgumentType                         = Arg;
    static constexpr uint16_t METHOD_MEMBER_ID = MemberIdValue;
    static constexpr bool COPY                 = CopyOnSend;
    static constexpr uint32_t TIMEOUT_VALUE    = TIMEOUT;
};

namespace internal
{

template<typename ArgumentType, typename Specialization = void>
struct CallbackHelper;

/**
 * Partial specialization of CallbackHelper with non-void ArgumentType.
 */
template<typename ArgumentType>
struct CallbackHelper<ArgumentType, etl::enable_if_t<(etl::is_void<ArgumentType>::value == false)>>
{
    using Callback = ::etl::delegate<void(etl::expected<ArgumentType, Future::State> const&)>;

    static void call(Callback const& callback, Message const& msg, Future::State const state)
    {
        if (state != Future::State::Ready)
        {
            callback.call_if(etl::unexpected<Future::State>(etl::in_place_t{}, state));
        }
        else
        {
            callback.call_if(etl::expected<ArgumentType, Future::State>(
                etl::in_place_t{},
                MessagePayloadBuilder::getInstance().readPayload<ArgumentType>(msg)));
        }
    }
};

/**
 * Partial specialization of CallbackHelper with void ArgumentType.
 */
template<typename ArgumentType>
struct CallbackHelper<ArgumentType, etl::enable_if_t<etl::is_void<ArgumentType>::value>>
{
    using Callback = ::etl::delegate<void(etl::expected<void, Future::State> const&&)>;

    static void call(Callback const& callback, Message const& /* msg */, Future::State const state)
    {
        if (state != Future::State::Ready)
        {
            callback.call_if(etl::unexpected<Future::State>(etl::in_place_t{}, state));
        }
        else
        {
            callback.call_if(etl::expected<void, Future::State>());
        }
    }
};

} // namespace internal

template<typename DispatcherTraits, uint8_t REQUEST_LIMIT = 1U>
class FutureDispatcher;

/**
 * Template class managing a pool of futures with callbacks for async request/response.
 *
 * \tparam DispatcherTraits Compile-time traits (ArgumentType, METHOD_MEMBER_ID, TIMEOUT_VALUE)
 * \tparam REQUEST_LIMIT Maximum number of concurrent in-flight requests
 */
template<typename DispatcherTraits, uint8_t REQUEST_LIMIT>
class FutureDispatcher : public FutureDispatcherBase
{
public:
    using Base         = FutureDispatcherBase;
    using ArgumentType = typename DispatcherTraits::ArgumentType;
    using Traits       = DispatcherTraits;
    using Callback     = typename internal::CallbackHelper<ArgumentType>::Callback;

    FutureDispatcher()
    : Base(
        _futuresArray,
        Base::CallbackDispatcher::
            template create<FutureDispatcher, &FutureDispatcher::triggerCallback>(*this))
    , _futuresArray()
    , _callbacks()
    {}

    /**
     * Registers a default callback to be used for all request slots.
     *
     * \param cbk The callback to assign to all request slots
     */
    void registerDefaultCallback(Callback const& cbk)
    {
        etl::fill(_callbacks.begin(), _callbacks.end(), cbk);
    }

    /**
     * Invokes the user callback associated with a specific future.
     *
     * \param msg The middleware message containing the response payload
     * \param future The future whose state has changed
     */
    void triggerCallback(Message const& msg, Future& future)
    {
        Callback const& cbk = _callbacks[etl::distance(_futuresArray.begin(), &future)];
        internal::CallbackHelper<ArgumentType>::call(cbk, msg, future.state);
    }

    /**
     * Allocates a request ID for a new request without timeout tracking.
     * Only enabled when TIMEOUT_VALUE == 0.
     *
     * \param callback User callback to invoke when the response arrives or an error occurs
     * \return RequestId on success, or etl::nullopt if no futures are available
     */
    template<uint32_t TIMEOUT = Traits::TIMEOUT_VALUE>
    typename etl::enable_if_t<TIMEOUT == 0U, etl::optional<uint16_t>>
    obtainRequestId(Callback const& callback)
    {
        uint16_t requestId{};
        etl::optional<Future*> const ret = Base::obtainRequestId(requestId);
        if (ret.has_value())
        {
            setCallback(etl::distance(_futuresArray.begin(), ret.value()), callback);
            return ret.value()->requestId;
        }
        return etl::nullopt;
    }

    /**
     * Allocates a request ID for a new request with timeout tracking.
     * Only enabled when TIMEOUT_VALUE > 0.
     *
     * \param callback User callback to invoke when the response arrives, timeout occurs, or an
     *                 error happens
     * \return RequestId on success, or etl::nullopt if no futures are available
     */
    template<uint32_t TIMEOUT = Traits::TIMEOUT_VALUE>
    typename etl::enable_if_t<(TIMEOUT > 0U), etl::optional<uint16_t>>
    obtainRequestId(Callback const& callback)
    {
        uint16_t requestId{};
        etl::optional<Future*> const ret = Base::obtainRequestId(requestId);
        if (ret.has_value())
        {
            setCallback(etl::distance(_futuresArray.begin(), ret.value()), callback);
            auto const now               = ::middleware::time::getCurrentTimeInMs();
            ret.value()->callerTimestamp = now;
            return ret.value()->requestId;
        }
        return etl::nullopt;
    }

    /**
     * Checks all pending requests for timeouts and invokes callbacks for expired requests.
     * Only enabled when TIMEOUT_VALUE > 0.
     */
    template<uint32_t TIMEOUT = Traits::TIMEOUT_VALUE>
    typename etl::enable_if_t<(TIMEOUT > 0U)> updateTimeouts()
    {
        auto const now = ::middleware::time::getCurrentTimeInMs();
        Base::updateTimeouts(now, Traits::TIMEOUT_VALUE);
    }

    /**
     * Processes a response message and invokes the corresponding user callback.
     *
     * \param msg The middleware message containing the response or error information
     */
    void answerRequest(Message const& msg)
    {
        Future* const future = futureMatchingRequestId(msg);
        if (future != nullptr)
        {
            internal::CallbackHelper<ArgumentType>::call(
                _callbacks[etl::distance(_futuresArray.begin(), future)], msg, future->state);
            future->state = Future::State::Invalid;
        }
    }

protected:
    /** Clears the callback at the specified index. */
    void unsetCallback(uint8_t const index) { _callbacks[index] = Callback(); }

    /** Sets the callback at the specified index. */
    void setCallback(uint8_t const index, Callback const callback) { _callbacks[index] = callback; }

private:
    etl::array<Future, REQUEST_LIMIT> _futuresArray;
    etl::array<Callback, REQUEST_LIMIT> _callbacks;
};

} // namespace core
} // namespace middleware
