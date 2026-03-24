// Copyright 2025 BMW AG

#include "middleware/core/ProxyBase.h"

#include "middleware/concurrency/LockStrategies.h"
#include "middleware/core/IClusterConnection.h"
#include "middleware/core/InstancesDatabase.h"
#include "middleware/core/LoggerApi.h"
#include "middleware/core/Message.h"
#include "middleware/core/types.h"
#include "middleware/logger/Logger.h"
#include "middleware/os/TaskIdProvider.h"

#include <etl/algorithm.h>
#include <etl/span.h>

#include <cstdint>

namespace middleware
{
namespace core
{

HRESULT
ProxyBase::sendMessage(Message& msg) const
{
    HRESULT res = HRESULT::NotRegistered;
    if (_connection != nullptr)
    {
        res = _connection->sendMessage(msg);
    }

    return res;
}

uint8_t ProxyBase::getSourceClusterId() const
{
    auto clusterId = static_cast<uint8_t>(INVALID_CLUSTER_ID);
    if (_connection != nullptr)
    {
        clusterId = _connection->getSourceClusterId();
    }
    return clusterId;
}

HRESULT
ProxyBase::initFromInstancesDatabase(
    uint16_t const instanceId,
    uint8_t const sourceCluster,
    etl::span<IInstanceDatabase const* const> const& dbRange)
{
    HRESULT ret = HRESULT::TransceiverInitializationFailed;
    unsubscribe(getServiceId());
    auto const* it = etl::find_if(
        dbRange.begin(),
        dbRange.end(),
        [instanceId](IInstanceDatabase const* const dataBase) -> bool
        {
            auto const instances = dataBase->getInstanceIdsRange();
            auto const* instanceIdIt
                = etl::lower_bound(instances.begin(), instances.end(), instanceId);
            return ((instanceIdIt != instances.end()) && ((*instanceIdIt) == instanceId));
        });
    if (it != dbRange.end())
    {
        auto const proxyCc = (*it)->getProxyConnectionsRange();
        auto const* ccIt   = etl::find_if(
            proxyCc.begin(),
            proxyCc.end(),
            [sourceCluster](IClusterConnection const* const clusConn)
            {
                if (clusConn != nullptr)
                {
                    return (clusConn->getSourceClusterId() == sourceCluster);
                }
                return false;
            });
        if (ccIt != proxyCc.end())
        {
            ret = (*ccIt)->subscribe(*this, instanceId);
            if ((HRESULT::Ok == ret) || (HRESULT::InstanceAlreadyRegistered == ret))
            {
                _connection = (*ccIt);
            }
        }
    }
    // only print error when configuration allows for it
    if ((HRESULT::Ok != ret) && (!dbRange.empty()))
    {
        logger::logInitFailure(
            logger::LogLevel::Critical,
            logger::Error::ProxyInitialization,
            ret,
            getServiceId(),
            instanceId,
            sourceCluster);
    }
    return ret;
}

void ProxyBase::unsubscribe(uint16_t const serviceId)
{
    if (_connection != nullptr)
    {
        _connection->unsubscribe(*this, serviceId);
        _connection = nullptr;
    }
}

Message ProxyBase::generateMessageHeader(uint16_t const memberId, uint16_t const requestId) const
{
    if (INVALID_REQUEST_ID != requestId)
    {
        return Message::createRequest(
            getServiceId(),
            memberId,
            requestId,
            getInstanceId(),
            _connection->getSourceClusterId(),
            _connection->getTargetClusterId(),
            getAddressId());
    }
    return Message::createFireAndForgetRequest(
        getServiceId(),
        memberId,
        getInstanceId(),
        _connection->getSourceClusterId(),
        _connection->getTargetClusterId());
}

uint8_t ProxyBase::getAddressId() const { return _addressId; }

bool ProxyBase::isInitialized() const { return (_connection != nullptr); }

void ProxyBase::setAddressId(uint8_t const addressId) { _addressId = addressId; }

void ProxyBase::checkCrossThreadError(uint32_t const initId) const
{
    if (ProxyBase::isInitialized())
    {
        auto const currentTaskId = ::middleware::os::getProcessId();
        if (initId != currentTaskId)
        {
            ::middleware::concurrency::suspendAllInterrupts();

            logger::logCrossThreadViolation(
                logger::LogLevel::Critical,
                logger::Error::ProxyCrossThreadViolation,
                getSourceClusterId(),
                getServiceId(),
                getInstanceId(),
                initId,
                currentTaskId);

            ETL_ASSERT_FAIL("Proxy cross thread violation detected.");
        }
    }
}

} // namespace core
} // namespace middleware
