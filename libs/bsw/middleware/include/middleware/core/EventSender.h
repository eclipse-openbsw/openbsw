// Copyright 2025 BMW AG

#pragma once

#include "middleware/core/Message.h"
#include "middleware/core/MessagePayloadBuilder.h"
#include "middleware/core/SkeletonBase.h"
#include "middleware/core/types.h"

#include <etl/expected.h>

#include <cstdint>

namespace middleware
{
namespace core
{

/**
 * Broadcasts typed and void events to all cluster connections of a skeleton.
 * An EventSender is owned by SkeletonAttribute and SkeletonEvent instances to send
 * change notifications and events to subscribed client proxies.
 */
class EventSender final
{
public:
    explicit EventSender(SkeletonBase& skeleton) : _skeleton(skeleton) {}

    ~EventSender()                                   = default;
    EventSender(EventSender const& other)            = delete;
    EventSender& operator=(EventSender const& other) = delete;
    EventSender(EventSender&& other)                 = delete;
    EventSender& operator=(EventSender&& other)      = delete;

    /**
     * Broadcasts a typed event to all cluster connections.
     * Creates an event message, allocates payload, and sends to all connected clusters.
     *
     * \tparam T Event data type
     * \param data Event data to broadcast
     * \param memberId Member identifier for the event
     * \return HRESULT Result code
     */
    template<typename T>
    [[nodiscard]] HRESULT send(T const& data, uint16_t const memberId) const
    {
        HRESULT ret = HRESULT::NotRegistered;
        if (_skeleton.isInitialized())
        {
            Message msg = Message::createEvent(
                _skeleton.getServiceId(),
                memberId,
                _skeleton.getInstanceId(),
                _skeleton.getSourceClusterId());
            ret = MessagePayloadBuilder::getInstance().allocate(
                data, msg, static_cast<uint8_t>(_skeleton.getClusterConnections().size()));
            if (ret == HRESULT::Ok)
            {
                ret = sendToClusters(msg);
            }
        }

        return ret;
    }

    /**
     * Broadcasts a void event (notification without data) to all cluster connections.
     *
     * \param memberId Member identifier for the event
     * \return HRESULT Result code
     */
    [[nodiscard]] HRESULT send(uint16_t memberId) const;

private:
    [[nodiscard]] HRESULT sendToClusters(Message& msg) const;

    SkeletonBase& _skeleton;
};

} // namespace core
} // namespace middleware
