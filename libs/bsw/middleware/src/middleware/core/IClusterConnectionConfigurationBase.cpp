// Copyright 2025 BMW AG

#include "middleware/core/IClusterConnectionConfigurationBase.h"

#include <etl/algorithm.h>
#include <etl/iterator.h>
#include <etl/vector.h>

#include "middleware/concurrency/LockStrategies.h"
#include "middleware/core/DatabaseManipulator.h"
#include "middleware/core/ITimeout.h"
#include "middleware/core/ITransceiver.h"
#include "middleware/core/Message.h"
#include "middleware/core/types.h"

namespace middleware
{
namespace core
{

void ITimeoutConfiguration::registerTimeoutTransceiver(
    ITimeout& transceiver, ::etl::ivector<ITimeout*>& timeoutTransceivers)
{
    auto it = etl::find(timeoutTransceivers.cbegin(), timeoutTransceivers.cend(), &transceiver);
    if (it == timeoutTransceivers.cend())
    {
        if (!timeoutTransceivers.full())
        {
            timeoutTransceivers.push_back(&transceiver);
        }
    }
}

void ITimeoutConfiguration::unregisterTimeoutTransceiver(
    ITimeout& transceiver, ::etl::ivector<ITimeout*>& timeoutTransceivers)
{
    using ETL_OR_STD::swap;

    auto it = etl::find(timeoutTransceivers.begin(), timeoutTransceivers.end(), &transceiver);
    if (it != timeoutTransceivers.cend())
    {
        swap(*it, timeoutTransceivers.back());
        timeoutTransceivers.pop_back();
    }
}

void ITimeoutConfiguration::updateTimeouts(::etl::ivector<ITimeout*> const& timeoutTransceivers)
{
    for (auto* const transceiver : timeoutTransceivers)
    {
        transceiver->updateTimeouts();
    }
}

HRESULT
IClusterConnectionConfigurationBase::dispatchMessageToProxy(
    meta::TransceiverContainer const* const proxiesStart,
    meta::TransceiverContainer const* const proxiesEnd,
    Message const& msg)
{
    HRESULT result = HRESULT::Ok;

    if (msg.isEvent())
    {
        auto const range = meta::DbManipulator::getTransceiversByServiceIdAndServiceInstanceId(
            proxiesStart, proxiesEnd, msg.getHeader().serviceId, msg.getHeader().serviceInstanceId);
        for (auto it = range.first; it != range.second; it = etl::next(it))
        {
            static_cast<void>((*it)->onNewMessageReceived(msg));
        }
    }
    else if (msg.isResponse())
    {
        Message::Header const& header   = msg.getHeader();
        ITransceiver* const transceiver = meta::DbManipulator::getTransceiver(
            proxiesStart, proxiesEnd, header.serviceId, header.serviceInstanceId, header.addressId);
        if (transceiver != nullptr)
        {
            result = transceiver->onNewMessageReceived(msg);
        }
    }
    else
    {
        result = HRESULT::RoutingError;
    }

    return result;
}

HRESULT
IClusterConnectionConfigurationBase::dispatchMessageToSkeleton(
    meta::TransceiverContainer const* const skeletonsStart,
    meta::TransceiverContainer const* const skeletonsEnd,
    Message const& msg)
{
    HRESULT result = HRESULT::Ok;

    if (msg.isRequest() || msg.isFireAndForgetRequest())
    {
        // message comes from proxy
        // dispatch to specific transceiver, identified by serviceInstanceId
        Message::Header const& header = msg.getHeader();
        auto skeleton = meta::DbManipulator::getSkeletonByServiceIdAndServiceInstanceId(
            skeletonsStart, skeletonsEnd, header.serviceId, header.serviceInstanceId);
        if (skeleton != nullptr)
        {
            result = skeleton->onNewMessageReceived(msg);
        }
        else
        {
            result = HRESULT::ServiceNotFound;
        }
    }
    else
    {
        result = HRESULT::RoutingError;
    }

    return result;
}

HRESULT IClusterConnectionConfigurationBase::dispatchMessage(
    meta::TransceiverContainer const* const proxiesStart,
    meta::TransceiverContainer const* const proxiesEnd,
    meta::TransceiverContainer const* const skeletonsStart,
    meta::TransceiverContainer const* const skeletonsEnd,
    Message const& msg)
{
    HRESULT result = HRESULT::Ok;
    if (msg.isEvent() || msg.isResponse())
    {
        result = dispatchMessageToProxy(proxiesStart, proxiesEnd, msg);
    }
    else
    {
        result = dispatchMessageToSkeleton(skeletonsStart, skeletonsEnd, msg);
    }

    return result;
}

} // namespace core
} // namespace middleware
