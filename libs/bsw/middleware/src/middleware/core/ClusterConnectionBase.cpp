// Copyright 2025 BMW AG

#include "middleware/core/ClusterConnectionBase.h"

#include <cstddef>
#include <cstdio>

#include "middleware/core/IClusterConnectionConfigurationBase.h"
#include "middleware/core/ITimeout.h"
#include "middleware/core/LoggerApi.h"
#include "middleware/core/Message.h"
// #include "middleware/core/MessageAllocator.h"
#include "middleware/core/types.h"
#include "middleware/logger/Logger.h"

namespace middleware
{
namespace core
{

ClusterConnectionBase::ClusterConnectionBase(IClusterConnectionConfigurationBase& configuration)
: fConfiguration(configuration)
{}

void ClusterConnectionBase::processMessage(Message const& msg) const
{
    static_cast<void>(dispatchMessage(msg));
    // MessageAllocator::deallocate(msg);
}

void ClusterConnectionBase::respondWithError(ErrorState const error, Message const& msg) const
{
    Message::Header const& header = msg.getHeader();
    if (header.requestId
        != INVALID_REQUEST_ID) // not a fire/forget method so send error response back
    {
        Message const errorResponse = Message::createErrorResponse(
            header.serviceId,
            header.memberId,
            header.requestId,
            header.serviceInstanceId,
            msg.getHeader().tgtClusterId,
            msg.getHeader().srcClusterId,
            msg.getHeader().addressId,
            error);
        static_cast<void>(sendMessage(errorResponse));
    }
}

HRESULT ClusterConnectionBase::sendMessage(Message const& msg) const
{
    auto res = HRESULT::QueueFull;
    if ((msg.getHeader().srcClusterId == msg.getHeader().tgtClusterId))
    {
        res = dispatchMessage(msg);
        // MessageAllocator::deallocate(msg);
    }
    else
    {
        if (fConfiguration.write(msg))
        {
            res = HRESULT::Ok;
        }
        else
        {
            logger::logMessageSendingFailure(
                logger::LogLevel::Error, logger::Error::SendMessage, res, msg);
        }
    }

    return res;
}

HRESULT ClusterConnectionBase::dispatchMessage(Message const& msg) const
{
    auto const res = fConfiguration.dispatchMessage(msg);
    if (HRESULT::Ok != res)
    {
        if (HRESULT::ServiceBusy == res)
        {
            // ServiceBusy can only occur when dispatching a get request a message to the Skeleton
            // side
            respondWithError(ErrorState::ServiceBusy, msg);
        }
        else if (HRESULT::ServiceNotFound == res)
        {
            // ServiceNotFound can only occur when dispatching a message to the Skeleton side
            respondWithError(ErrorState::ServiceNotFound, msg);
        }
        else
        {
            // other use cases are not relevant because they won't happen on the message dispatching
        }

        logger::logMessageSendingFailure(
            logger::LogLevel::Error, logger::Error::DispatchMessage, res, msg);
    }

    return res;
}

ClusterConnectionTimeoutBase::ClusterConnectionTimeoutBase(ITimeoutConfiguration& configuration)
: ClusterConnectionBase(configuration)
{}

void ClusterConnectionTimeoutBase::registerTimeoutTransceiver(ITimeout& transceiver)
{
    static_cast<ITimeoutConfiguration&>((ClusterConnectionBase::getConfiguration()))
        .registerTimeoutTransceiver(transceiver);
}

void ClusterConnectionTimeoutBase::unregisterTimeoutTransceiver(ITimeout& transceiver)
{
    static_cast<ITimeoutConfiguration&>((ClusterConnectionBase::getConfiguration()))
        .unregisterTimeoutTransceiver(transceiver);
}

void ClusterConnectionTimeoutBase::updateTimeouts()
{
    static_cast<ITimeoutConfiguration&>((ClusterConnectionBase::getConfiguration()))
        .updateTimeouts();
}

} // namespace core
} // namespace middleware
