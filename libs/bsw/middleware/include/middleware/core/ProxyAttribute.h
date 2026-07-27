/********************************************************************************
 * Copyright (c) 2025, 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "middleware/core/Message.h"
#include "middleware/core/ProxyAttributeBase.h"
#include "middleware/core/ProxyEventBase.h"
#include "middleware/core/types.h"
#include "middleware/rpc/ProxyFireAndForgetMethod.h"
#include "middleware/rpc/ProxyMethod.h"

#include <etl/optional.h>
#include <etl/type_traits.h>

#include <cstdint>

namespace middleware::core
{

/// Helper to combine multiple base classes via inheritance.
template<typename... T>
struct InheritanceDelegate : T...
{
    InheritanceDelegate(ProxyBase& proxy) : T(proxy)... {}
};

/// Enumeration of attribute configurations for proxy attributes.
enum class AttributeType : uint8_t
{
    /// with get()
    ReadOnly_NoSubscription,
    /// with get(), setReceiveHandler(handler)
    ReadOnly,
    /// with get(), set(payload)
    NoSubscriptions,
    /// with get(), set(payload), setReceiveHandler(handler)
    FullyFeatured,
    /// with get(), set(payload, callback)
    NoSubscriptions_SetAsMethod,
    /// with get(), set(payload, callback), setReceiveHandler(handler)
    FullyFeatured_SetAsMethod
};

/**
 * Selects the appropriate base class for a ProxyAttribute based on its AttributeType.
 */
template<
    typename Proxy,
    AttributeType Type,
    typename ValueType,
    typename GetterTraits   = void,
    typename Specialization = void>
struct ProxyAttributeBaseSelector;

template<typename Proxy, AttributeType Type, typename ValueType, typename GetterTraits>
struct ProxyAttributeBaseSelector<
    Proxy,
    Type,
    ValueType,
    GetterTraits,
    typename ::etl::enable_if<
        (Type == AttributeType::ReadOnly_NoSubscription) || (Type == AttributeType::NoSubscriptions)
        || (Type == AttributeType::NoSubscriptions_SetAsMethod)>::type>
{
    using type = ProxyAttributeBase<GetterTraits>;
};

template<typename Proxy, AttributeType Type, typename ValueType, typename GetterTraits>
struct ProxyAttributeBaseSelector<
    Proxy,
    Type,
    ValueType,
    GetterTraits,
    typename ::etl::enable_if<
        (Type == AttributeType::ReadOnly) || (Type == AttributeType::FullyFeatured)
        || (Type == AttributeType::FullyFeatured_SetAsMethod)>::type>
{
    using type
        = InheritanceDelegate<ProxyAttributeBase<GetterTraits>, ProxyEventBase<Proxy, ValueType>>;
};

// ---- ProxyAttribute: fire-and-forget set variants ----

template<
    typename Proxy,
    typename GetterTraits,
    typename SetterTraits,
    AttributeType Type,
    typename ValueType,
    typename Specialization = void>
class ProxyAttribute;

/**
 * ProxyAttribute specialization for fire-and-forget set (no setter callback).
 * Covers ReadOnly_NoSubscription, ReadOnly, NoSubscriptions, FullyFeatured.
 */
template<
    typename Proxy,
    typename GetterTraits,
    typename SetterTraits,
    AttributeType Type,
    typename ValueType>
class ProxyAttribute<
    Proxy,
    GetterTraits,
    SetterTraits,
    Type,
    ValueType,
    typename ::etl::enable_if<
        (Type == AttributeType::ReadOnly_NoSubscription) || (Type == AttributeType::ReadOnly)
        || (Type == AttributeType::NoSubscriptions)
        || (Type == AttributeType::FullyFeatured)>::type>
: public ProxyAttributeBaseSelector<Proxy, Type, ValueType, GetterTraits>::type
{
public:
    using Base = typename ProxyAttributeBaseSelector<Proxy, Type, ValueType, GetterTraits>::type;
    using GetterMethodTraits = GetterTraits;

    explicit ProxyAttribute(ProxyBase& proxy) : Base(proxy) {}

    /**
     * Sends a fire-and-forget setter request.
     * Only enabled for NoSubscriptions and FullyFeatured attribute types.
     *
     * \param payload  Setter payload to serialize into the message
     * \param methodId The member ID identifying the setter on the wire
     * \return RequestId on success, or HRESULT on failure
     */
    template<AttributeType T = Type>
    typename ::etl::enable_if<
        (T == AttributeType::NoSubscriptions) || (T == AttributeType::FullyFeatured),
        ::etl::expected<uint16_t, HRESULT>>::type
    set(ValueType const& payload, uint16_t methodId)
    {
        return rpc::ProxyFireAndForgetMethod().callMethod(*Base::_proxy, payload, methodId);
    }
};

// ---- ProxyAttribute: set-as-method variants (with setter callback) ----

/**
 * ProxyAttribute specialization for set-as-method (setter has its own future dispatcher).
 * Covers NoSubscriptions_SetAsMethod, FullyFeatured_SetAsMethod.
 */
template<
    typename Proxy,
    typename GetterTraits,
    typename SetterTraits,
    AttributeType Type,
    typename ValueType>
class ProxyAttribute<
    Proxy,
    GetterTraits,
    SetterTraits,
    Type,
    ValueType,
    typename ::etl::enable_if<
        (Type == AttributeType::NoSubscriptions_SetAsMethod)
        || (Type == AttributeType::FullyFeatured_SetAsMethod)>::type>
: public ProxyAttributeBaseSelector<Proxy, Type, ValueType, GetterTraits>::type
{
public:
    using Base = typename ProxyAttributeBaseSelector<Proxy, Type, ValueType, GetterTraits>::type;
    using SetterMethodTraits = SetterTraits;
    using SetterMethod       = rpc::ProxyMethod<SetterMethodTraits>;
    using SetterCallback     = typename SetterMethod::Callback;
    using SetterResult       = typename SetterMethod::Result;
    using GetterMethodTraits = GetterTraits;
    using GetterResult       = typename Base::GetterResult;
    using GetterCallback     = typename Base::GetterCallback;

    explicit ProxyAttribute(ProxyBase& proxy) : Base(proxy) {}

    /**
     * Dispatches an incoming getter response message to the getter dispatcher.
     */
    typename ::etl::enable_if<!::etl::is_void<GetterMethodTraits>::value, void>::type
    answerGetterRequest(Message const& msg)
    {
        Base::answerGetterRequest(msg);
    }

    /**
     * Dispatches a setter response message to the setter dispatcher.
     */
    void answerSetterRequest(Message const& msg) { _setMethod.answerRequest(msg); }

    /**
     * Cancels an in-flight getter request.
     */
    typename ::etl::enable_if<!::etl::is_void<GetterMethodTraits>::value, HRESULT>::type
    cancelGetterRequest(uint16_t const requestId)
    {
        return Base::cancelGetterRequest(requestId);
    }

    /**
     * Cancels an in-flight setter request.
     */
    HRESULT cancelSetterRequest(uint16_t const requestId)
    {
        return _setMethod.cancelRequest(requestId);
    }

    /**
     * Updates timeout tracking for setter and getter dispatchers when enabled.
     */
    template<typename T = GetterMethodTraits>
    typename ::etl::enable_if<(!::etl::is_void<T>::value) && (SetterTraits::TIMEOUT != 0U), void>::
        type
        updateTimeouts()
    {
        _setMethod.updateTimeouts();
        Base::updateTimeouts();
    }

    /**
     * Updates timeout tracking for setter dispatcher only when enabled.
     */
    template<typename T = GetterMethodTraits>
    typename ::etl::enable_if<(::etl::is_void<T>::value) && (SetterTraits::TIMEOUT != 0U), void>::
        type
        updateTimeouts()
    {
        _setMethod.updateTimeouts();
    }

    /**
     * Releases all in-flight setter and getter requests.
     */
    template<typename T = GetterMethodTraits>
    typename ::etl::enable_if<!::etl::is_void<T>::value, void>::type freeAll()
    {
        _setMethod.freeAll();
        Base::freeAll();
    }

    /**
     * Releases all in-flight setter requests when no getter interface is present.
     */
    template<typename T = GetterMethodTraits>
    typename ::etl::enable_if<::etl::is_void<T>::value, void>::type freeAll()
    {
        _setMethod.freeAll();
    }

    /**
     * Sends a setter request with callback-based response handling.
     *
     * \param payload  Setter argument payload
     * \param methodId The member ID identifying the setter on the wire
     * \param cbk      Callback invoked when the response is received or fails
     * \return RequestId on success, or HRESULT on failure
     */
    ::etl::expected<uint16_t, HRESULT>
    set(ValueType const& payload, uint16_t methodId, SetterCallback const cbk)
    {
        return _setMethod.callMethod(*Base::_proxy, payload, methodId, cbk);
    }

private:
    SetterMethod _setMethod;
};

} // namespace middleware::core
