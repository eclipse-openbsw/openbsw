// Copyright 2025 BMW AG

#pragma once

#include "middleware/core/Message.h"
#include "middleware/core/MessagePayloadBuilder.h"
#include "middleware/core/ProxyAttributeBase.h"
#include "middleware/core/ProxyEventBase.h"
#include "middleware/core/types.h"

#include <etl/optional.h>
#include <etl/type_traits.h>

#include <cstdint>

namespace middleware
{
namespace core
{

/// Helper to combine multiple base classes via inheritance.
template<typename... T>
struct InheritanceDelegate : T...
{
    InheritanceDelegate(ProxyBase& proxy) : T(proxy)... {};
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
    uint8_t REQUEST_LIMIT   = 0U,
    typename Specialization = void>
struct ProxyAttributeBaseSelector;

template<
    typename Proxy,
    AttributeType Type,
    typename ValueType,
    typename GetterTraits,
    uint8_t REQUEST_LIMIT>
struct ProxyAttributeBaseSelector<
    Proxy,
    Type,
    ValueType,
    GetterTraits,
    REQUEST_LIMIT,
    typename etl::enable_if<
        (Type == AttributeType::ReadOnly_NoSubscription) || (Type == AttributeType::NoSubscriptions)
        || (Type == AttributeType::NoSubscriptions_SetAsMethod)>::type>
{
    using type = ProxyAttributeBase<REQUEST_LIMIT, GetterTraits>;
};

template<
    typename Proxy,
    AttributeType Type,
    typename ValueType,
    typename GetterTraits,
    uint8_t REQUEST_LIMIT>
struct ProxyAttributeBaseSelector<
    Proxy,
    Type,
    ValueType,
    GetterTraits,
    REQUEST_LIMIT,
    typename etl::enable_if<
        (Type == AttributeType::ReadOnly) || (Type == AttributeType::FullyFeatured)
        || (Type == AttributeType::FullyFeatured_SetAsMethod)>::type>
{
    using type = InheritanceDelegate<
        ProxyAttributeBase<REQUEST_LIMIT, GetterTraits>,
        ProxyEventBase<Proxy, ValueType>>;
};

// ---- ProxyAttribute: fire-and-forget set variants ----

template<
    typename Proxy,
    typename GetterTraits,
    uint8_t REQUEST_LIMIT,
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
    uint8_t REQUEST_LIMIT,
    typename SetterTraits,
    AttributeType Type,
    typename ValueType>
class ProxyAttribute<
    Proxy,
    GetterTraits,
    REQUEST_LIMIT,
    SetterTraits,
    Type,
    ValueType,
    typename etl::enable_if<
        (Type == AttributeType::ReadOnly_NoSubscription) || (Type == AttributeType::ReadOnly)
        || (Type == AttributeType::NoSubscriptions)
        || (Type == AttributeType::FullyFeatured)>::type>
: public ProxyAttributeBaseSelector<Proxy, Type, ValueType, GetterTraits, REQUEST_LIMIT>::type
{
public:
    using Base =
        typename ProxyAttributeBaseSelector<Proxy, Type, ValueType, GetterTraits, REQUEST_LIMIT>::
            type;
    using GetterDispatcherTraits = GetterTraits;

    explicit ProxyAttribute(ProxyBase& proxy) : Base(proxy) {}

    /**
     * Sends a fire-and-forget setter request.
     * Only enabled for NoSubscriptions and FullyFeatured attribute types.
     *
     * \param payload Setter payload to serialize into the message
     * \return RequestId on success, or HRESULT on failure
     */
    template<AttributeType T = Type>
    typename etl::enable_if<
        (T == AttributeType::NoSubscriptions) || (T == AttributeType::FullyFeatured),
        etl::expected<uint16_t, HRESULT>>::type
    set(ValueType const& payload)
    {
        HRESULT ret = HRESULT::NotRegistered;
        if (Base::_proxy->isInitialized())
        {
            Message msg = Base::_proxy->generateMessageHeader(SetterTraits::METHOD_MEMBER_ID);
            ret         = MessagePayloadBuilder::getInstance().allocate(payload, msg);
            if (ret == HRESULT::Ok)
            {
                ret = Base::_proxy->sendMessage(msg);
                if (ret != HRESULT::Ok)
                {
                    MessagePayloadBuilder::deallocate(msg);
                    return etl::unexpected<HRESULT>(etl::in_place_t{}, ret);
                }
                return msg.getHeader().requestId;
            }
        }

        return etl::unexpected<HRESULT>(etl::in_place_t{}, ret);
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
    uint8_t REQUEST_LIMIT,
    typename SetterTraits,
    AttributeType Type,
    typename ValueType>
class ProxyAttribute<
    Proxy,
    GetterTraits,
    REQUEST_LIMIT,
    SetterTraits,
    Type,
    ValueType,
    typename etl::enable_if<
        (Type == AttributeType::NoSubscriptions_SetAsMethod)
        || (Type == AttributeType::FullyFeatured_SetAsMethod)>::type>
: public ProxyAttributeBaseSelector<Proxy, Type, ValueType, GetterTraits, REQUEST_LIMIT>::type
{
public:
    using Base =
        typename ProxyAttributeBaseSelector<Proxy, Type, ValueType, GetterTraits, REQUEST_LIMIT>::
            type;
    using SetterArgumentType     = typename SetterTraits::ArgumentType;
    using SetterCallback         = typename FutureDispatcher<SetterTraits, REQUEST_LIMIT>::Callback;
    using GetterDispatcherTraits = GetterTraits;
    using GetterCallback         = typename Base::GetterCallback;

    explicit ProxyAttribute(ProxyBase& proxy) : Base(proxy) {}

    /**
     * Dispatches an incoming getter response message to the getter dispatcher.
     */
    typename etl::enable_if<!etl::is_void<GetterTraits>::value, void>::type
    answerGetterRequest(Message const& msg)
    {
        Base::answerGetterRequest(msg);
    }

    /**
     * Dispatches a setter response message to the setter dispatcher.
     */
    void answerSetterRequest(Message const& msg) { _setDispatcher.answerRequest(msg); }

    /**
     * Cancels an in-flight getter request.
     */
    typename etl::enable_if<!etl::is_void<GetterTraits>::value, HRESULT>::type
    cancelGetterRequest(uint16_t const requestId)
    {
        return Base::cancelGetterRequest(requestId);
    }

    /**
     * Cancels an in-flight setter request.
     */
    HRESULT cancelSetterRequest(uint16_t const requestId)
    {
        return _setDispatcher.cancelRequest(requestId);
    }

    /**
     * Cancels an in-flight setter request when no getter interface is present.
     */
    template<typename T = GetterTraits>
    typename etl::enable_if<etl::is_void<T>::value, HRESULT>::type
    cancelRequest(uint16_t const reqId)
    {
        return _setDispatcher.cancelRequest(reqId);
    }

    /**
     * Updates timeout tracking for setter and getter dispatchers when enabled.
     */
    template<typename T = GetterTraits>
    typename etl::
        enable_if<(!etl::is_void<T>::value) && (SetterTraits::TIMEOUT_VALUE != 0U), void>::type
        updateTimeouts()
    {
        _setDispatcher.updateTimeouts();
        Base::updateTimeouts();
    }

    /**
     * Updates timeout tracking for setter dispatcher only when enabled.
     */
    template<typename T = GetterTraits>
    typename etl::enable_if<(etl::is_void<T>::value) && (SetterTraits::TIMEOUT_VALUE != 0U), void>::
        type
        updateTimeouts()
    {
        _setDispatcher.updateTimeouts();
    }

    /**
     * Releases all in-flight setter and getter requests.
     */
    template<typename T = GetterTraits>
    typename etl::enable_if<!etl::is_void<T>::value, void>::type freeAll()
    {
        _setDispatcher.freeAll();
        Base::freeAll();
    }

    /**
     * Releases all in-flight setter requests when no getter interface is present.
     */
    template<typename T = GetterTraits>
    typename etl::enable_if<etl::is_void<T>::value, void>::type freeAll()
    {
        _setDispatcher.freeAll();
    }

    /**
     * Sends a setter request with callback-based response handling.
     *
     * \param payload Setter argument payload
     * \param cbk Callback invoked when the response is received or fails
     * \return RequestId on success, or HRESULT on failure
     */
    etl::expected<uint16_t, HRESULT>
    set(SetterArgumentType const& payload, SetterCallback const& cbk)
    {
        etl::expected<uint16_t, HRESULT> result
            = etl::unexpected<HRESULT>(etl::in_place_t{}, HRESULT::NotRegistered);

        if (Base::_proxy->isInitialized())
        {
            etl::optional<uint16_t> reqId = _setDispatcher.obtainRequestId(cbk);
            if (reqId.has_value())
            {
                Message msg = Base::_proxy->generateMessageHeader(
                    SetterTraits::METHOD_MEMBER_ID, reqId.value());
                HRESULT res = MessagePayloadBuilder::getInstance().allocate(payload, msg);
                if (res == HRESULT::Ok)
                {
                    res = Base::_proxy->sendMessage(msg);
                    if (res == HRESULT::Ok)
                    {
                        result = reqId.value();
                    }
                    else
                    {
                        MessagePayloadBuilder::deallocate(msg);
                        logger::logMessageSendingFailure(
                            logger::LogLevel::Error, logger::Error::SendMessage, res, msg);
                        result = etl::unexpected<HRESULT>(etl::in_place_t{}, res);
                    }
                }
                else
                {
                    result = etl::unexpected<HRESULT>(etl::in_place_t{}, res);
                }

                if (!result.has_value())
                {
                    static_cast<void>(_setDispatcher.cancelRequest(reqId.value()));
                }
            }
            else
            {
                result = etl::unexpected<HRESULT>(etl::in_place_t{}, HRESULT::RequestPoolDepleted);
            }
        }

        return result;
    }

private:
    FutureDispatcher<SetterTraits, REQUEST_LIMIT> _setDispatcher;
};

} // namespace core
} // namespace middleware
