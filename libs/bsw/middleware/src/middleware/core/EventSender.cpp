// Copyright 2025 BMW AG

#include "middleware/core/EventSender.h"

#include "middleware/core/IClusterConnection.h"
#include "middleware/core/Message.h"
#include "middleware/core/MessagePayloadBuilder.h"
#include "middleware/core/SkeletonBase.h"
#include "middleware/core/types.h"

namespace middleware::core
{

HRESULT EventSender::send(uint16_t const memberId) const
{
    HRESULT ret = HRESULT::NotRegistered;
    if (_skeleton.isInitialized())
    {
        Message msg = Message::createEvent(
            _skeleton.getServiceId(),
            memberId,
            _skeleton.getInstanceId(),
            _skeleton.getSourceClusterId());
        ret = sendToClusters(msg);
    }

    return ret;
}

HRESULT EventSender::sendToClusters(Message& msg) const
{
    HRESULT ret                    = HRESULT::Ok;
    auto const& clusterConnections = _skeleton.getClusterConnections();
    _skeleton.checkCrossThreadError(_skeleton.getProcessId());
    for (IClusterConnection* const connection : clusterConnections)
    {
        if (connection->sendMessage(msg) != HRESULT::Ok)
        {
            MessagePayloadBuilder::deallocate(msg);
            ret = HRESULT::EventNotSendSuccessfully;
        }
    }

    return ret;
}

} // namespace middleware::core
