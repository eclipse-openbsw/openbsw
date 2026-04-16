// Copyright 2025 BMW AG

#pragma once

#include "middleware/core/Message.h"
#include "middleware/core/MessagePayloadBuilder.h"

#include <etl/delegate.h>
#include <etl/type_traits.h>

namespace middleware::core
{

/**
 * Type selector for event callback signatures.
 * For typed events, the callback takes a const reference to the event type.
 * For void events, the callback takes no arguments.
 *
 * \tparam EventType The type of the event data (or void for notification-only events)
 */
template<typename EventType>
struct EventCallbackTypeSelector
{
    using Callback = ::etl::delegate<void(EventType const&)>;
};

template<>
struct EventCallbackTypeSelector<void>
{
    using Callback = ::etl::delegate<void()>;
};

/**
 * Base class for proxy event subscription handling.
 * Manages registration and invocation of event callbacks on the proxy side.
 *
 * \tparam Proxy The concrete proxy class (CRTP friend)
 * \tparam EventType The event data type (or void for notification-only events)
 */
template<class Proxy, typename EventType>
class ProxyEventBase
{
    friend Proxy;

public:
    using OnFieldChangedCallback = typename EventCallbackTypeSelector<EventType>::Callback;
    using Callback               = OnFieldChangedCallback;

    explicit ProxyEventBase(ProxyBase& /* proxy */) : _cbk() {}

    ~ProxyEventBase() noexcept { unsetReceiveHandler(); }

    ProxyEventBase& operator=(ProxyEventBase const&) = delete;
    ProxyEventBase(ProxyEventBase const&)            = delete;
    ProxyEventBase& operator=(ProxyEventBase&&)      = delete;
    ProxyEventBase(ProxyEventBase&&)                 = delete;

    /**
     * Registers a callback function to be invoked when the subscribed event is broadcast.
     *
     * \param callback Event notification callback
     */
    void setReceiveHandler(OnFieldChangedCallback const callback) noexcept { _cbk = callback; }

    /**
     * Unregisters the event callback.
     * After calling this method, no callback will be invoked when events are received.
     */
    void unsetReceiveHandler() noexcept { _cbk = OnFieldChangedCallback(); }

private:
    template<typename E = EventType, typename etl::enable_if<etl::is_void<E>::value, int>::type = 0>
    void setEvent_(Message const& /* msg */) const
    {
        _cbk.call_if();
    }

    template<
        typename E                                                  = EventType,
        typename etl::enable_if<!etl::is_void<E>::value, int>::type = 0>
    void setEvent_(Message const& msg) const
    {
        E const& data = MessagePayloadBuilder::getInstance().template readPayload<E>(msg);
        _cbk.call_if(data);
    }

    OnFieldChangedCallback _cbk;
};

} // namespace middleware::core
