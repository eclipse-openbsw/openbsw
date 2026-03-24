// Copyright 2025 BMW AG

#pragma once

#include "middleware/core/EventSender.h"
#include "middleware/core/SkeletonBase.h"
#include "middleware/core/types.h"

#include <etl/span.h>
#include <etl/type_traits.h>
#include <etl/utility.h>

#include <cstdint>

namespace middleware
{
namespace core
{

/**
 * Skeleton-side event broadcaster for typed events.
 *
 * \tparam Type The event data type
 * \tparam MEMBER_ID The member ID associated with this event
 */
template<typename Type, uint16_t MEMBER_ID>
class SkeletonEvent
{
public:
    using EventType = Type;

    SkeletonEvent(SkeletonBase& skeleton) : _eventSender(skeleton) {}

    /**
     * Broadcasts an event with typed data to all subscribed client proxies.
     *
     * \param data Event data to broadcast
     * \return HRESULT Result code
     */
    [[nodiscard]] HRESULT send(Type const& data) const
    {
        return _eventSender.send<EventType>(data, MEMBER_ID);
    }

private:
    EventSender _eventSender;
};

/**
 * Partial specialization for void events (notification without data).
 *
 * \tparam MEMBER_ID The member ID associated with this event
 */
template<uint16_t MEMBER_ID>
class SkeletonEvent<void, MEMBER_ID>
{
public:
    using EventType = void;

    SkeletonEvent(SkeletonBase& skeleton) : _eventSender(skeleton) {}

    /**
     * Broadcasts a void event (notification without data) to all subscribed proxies.
     *
     * \return HRESULT Result code
     */
    [[nodiscard]] HRESULT send() const { return _eventSender.send(MEMBER_ID); }

private:
    EventSender _eventSender;
};

} // namespace core
} // namespace middleware
