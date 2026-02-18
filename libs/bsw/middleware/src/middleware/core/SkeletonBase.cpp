// Copyright 2025 BMW AG

#include "middleware/core/SkeletonBase.h"

#include <cstddef>
#include <cstdint>

#include <etl/algorithm.h>
#include <etl/span.h>

#include "middleware/concurrency/LockStrategies.h"
#include "middleware/core/IClusterConnection.h"
#include "middleware/core/InstancesDatabase.h"
#include "middleware/core/LoggerApi.h"
#include "middleware/core/Message.h"
#include "middleware/core/types.h"
#include "middleware/logger/Logger.h"
#include "middleware/os/TaskIdProvider.h"

namespace middleware
{
namespace core
{

HRESULT
SkeletonBase::sendMessage(Message& msg) const
{
    HRESULT res = HRESULT::ClusterIdNotFoundOrTransceiverNotRegistered;
    auto sender = etl::find_if(
        connections_.begin(),
        connections_.end(),
        [&msg](IClusterConnection const* const clusConn)
        {
            if (clusConn != nullptr)
            {
                return (clusConn->getTargetClusterId() == msg.getHeader().tgtClusterId);
            }
            return false;
        });

    if (sender != connections_.end())
    {
        res = (*sender)->sendMessage(msg);
    }
    else
    {
        logger::logMessageSendingFailure(
            logger::LogLevel::Error, logger::Error::SendMessage, res, msg);
    }

    return res;
}

uint8_t SkeletonBase::getSourceClusterId() const
{
    auto clusterId = static_cast<uint8_t>(INVALID_CLUSTER_ID);
    if (!connections_.empty())
    {
        auto it = etl::find_if(
            connections_.begin(),
            connections_.end(),
            [](IClusterConnection const* const clusConn) { return (clusConn != nullptr); });

        if (it != connections_.end())
        {
            clusterId = (*it)->getSourceClusterId();
        }
    }
    return clusterId;
}

void SkeletonBase::unsubscribe(uint16_t const serviceId)
{
    if (nullptr != connections_.data())
    {
        for (auto* const connection : connections_)
        {
            if (connection != nullptr)
            {
                connection->unsubscribe(*this, serviceId);
            }
        }
    }
    connections_ = etl::span<IClusterConnection*>();
}

etl::span<IClusterConnection* const> const& SkeletonBase::getClusterConnections() const
{
    return connections_;
}

bool SkeletonBase::isInitialized() const { return (!connections_.empty()); }

HRESULT
SkeletonBase::initFromInstancesDatabase(
    uint16_t const instanceId, etl::span<IInstanceDatabase const* const> const& dbRange)
{
    unsubscribe(getServiceId());
    auto it = etl::find_if(
        dbRange.begin(),
        dbRange.end(),
        [instanceId](IInstanceDatabase const* const dataBase) -> bool
        {
            auto const instances = dataBase->getInstanceIdsRange();
            auto instanceIdIt    = etl::lower_bound(instances.begin(), instances.end(), instanceId);
            return ((instanceIdIt != instances.end()) && ((*instanceIdIt) == instanceId));
        });
    HRESULT ret = HRESULT::TransceiverInitializationFailed;
    if (it != dbRange.end())
    {
        auto skeletonCc = (*it)->getSkeletonConnectionsRange();
        if (skeletonCc.empty())
        {
            instanceId_ = INVALID_INSTANCE_ID;
            ret         = HRESULT::NoClientsAvailable;
        }
        else
        {
            bool isRegistered = true;
            for (auto* const clusConn : skeletonCc)
            {
                if (nullptr != clusConn)
                {
                    ret = clusConn->subscribe(*this, instanceId);
                    if ((ret == HRESULT::Ok) || (ret == HRESULT::InstanceAlreadyRegistered))
                    {
                        continue;
                    }

                    isRegistered = false;
                    break;
                }
            }
            if (isRegistered)
            {
                connections_ = skeletonCc;
            }
            else
            {
                unsubscribe(getServiceId());
                instanceId_ = INVALID_INSTANCE_ID;
                ret         = HRESULT::TransceiverInitializationFailed;
            }
        }
    }
    else
    {
        ret = HRESULT::InstanceNotFound;
    }

    if (HRESULT::Ok != ret)
    {
        logger::logInitFailure(
            logger::LogLevel::Critical,
            logger::Error::SkeletonInitialization,
            ret,
            getServiceId(),
            instanceId,
            INVALID_CLUSTER_ID);
    }
    return ret;
}

SkeletonBase::~SkeletonBase() = default;

void SkeletonBase::checkCrossThreadError(uint32_t const initId) const
{
    if (SkeletonBase::isInitialized())
    {
        auto const currentTaskId = ::middleware::os::getProcessId();
        if (initId != currentTaskId)
        {
            ::middleware::concurrency::suspendAllInterrupts();

            logger::logCrossThreadViolation(
                logger::LogLevel::Critical,
                logger::Error::SkeletonCrossThreadViolation,
                getSourceClusterId(),
                getServiceId(),
                getInstanceId(),
                initId,
                currentTaskId);

            ETL_ASSERT_FAIL("Skeleton cross thread violation detected.");
        }
    }
}

} // namespace core
} // namespace middleware
