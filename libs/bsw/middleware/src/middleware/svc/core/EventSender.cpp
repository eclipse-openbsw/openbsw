/********************************************************************************
 * Copyright (c) 2025 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "middleware/svc/core/EventSender.h"

#include "middleware/svc/core/IClusterConnection.h"
#include "middleware/svc/core/Message.h"
#include "middleware/svc/core/MessagePayloadBuilder.h"
#include "middleware/svc/core/SkeletonBase.h"
#include "middleware/svc/core/types.h"
#include <cstdint>

namespace middleware::svc::core
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

} // namespace middleware::svc::core
