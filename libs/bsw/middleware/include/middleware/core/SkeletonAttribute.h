// Copyright 2025 BMW AG

#pragma once

#include "middleware/core/EventSender.h"
#include "middleware/core/SkeletonBase.h"
#include "middleware/core/types.h"

#include <etl/type_traits.h>

#include <cstdint>

namespace middleware::core
{

/**
 * Skeleton-side attribute that stores a value and optionally broadcasts changes to proxies.
 *
 * \tparam T The attribute value type
 * \tparam MEMBER_ID The member ID associated with this attribute
 * \tparam AllowsSubscriptions Whether sending change notifications is enabled
 */
template<typename T, uint16_t MEMBER_ID, bool AllowsSubscriptions>
class SkeletonAttribute
{
public:
    using AttributeType = T;

    SkeletonAttribute(SkeletonBase& skeleton) : _eventSender(skeleton) {}

    /** Returns a const reference to the current attribute value. */
    AttributeType const& get() const { return _attributeValue; }

    /** Returns a mutable reference to the current attribute value. */
    AttributeType& get() { return _attributeValue; }

    /**
     * Updates the stored attribute value without broadcasting to clients.
     *
     * \param value New attribute value
     */
    void set(AttributeType const& value)
    {
        if (&value != &_attributeValue)
        {
            _attributeValue = value;
        }
    }

    /**
     * Updates the attribute value and broadcasts it to all subscribed client proxies.
     * Only enabled when AllowsSubscriptions is true.
     *
     * \param attribute New attribute value to store and broadcast
     * \return HRESULT Result code
     */
    template<bool B = AllowsSubscriptions, typename = etl::enable_if_t<B>>
    [[nodiscard]] HRESULT send(AttributeType const& attribute)
    {
        set(attribute);
        return _eventSender.send(attribute, MEMBER_ID);
    }

    /**
     * Broadcasts the current attribute value to all subscribed client proxies.
     * Only enabled when AllowsSubscriptions is true.
     *
     * \return HRESULT Result code
     */
    template<bool B = AllowsSubscriptions, typename = etl::enable_if_t<B>>
    [[nodiscard]] HRESULT send() const
    {
        return _eventSender.send(get(), MEMBER_ID);
    }

private:
    EventSender _eventSender;
    AttributeType _attributeValue{};
};

} // namespace middleware::core
