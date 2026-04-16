// Copyright 2025 BMW AG

#include "middleware/core/DatabaseManipulator.h"

#include "middleware/concurrency/LockStrategies.h"
#include "middleware/core/ProxyBase.h"
#include "middleware/core/SkeletonBase.h"
#include "middleware/core/TransceiverBase.h"
#include "middleware/core/TransceiverContainer.h"
#include "middleware/core/types.h"

#include <etl/algorithm.h>
#include <etl/utility.h>
#include <etl/vector.h>

#include <cstddef>
#include <cstdint>

namespace middleware::core::meta
{

HRESULT
DbManipulator::subscribe(
    middleware::core::meta::TransceiverContainer* const start,
    middleware::core::meta::TransceiverContainer* const end,
    ProxyBase& proxy,
    uint16_t const instanceId,
    uint16_t const maxServiceId)
{
    auto res = HRESULT::ServiceNotFound;
    if (proxy.getServiceId() > maxServiceId)
    {
        res = HRESULT::ServiceIdOutOfRange;
    }
    else
    {
        auto const serviceId = proxy.getServiceId();
        auto* containerIt    = getTransceiversByServiceId(start, end, serviceId);
        if (containerIt != end)
        {
            auto& container = *containerIt->_container;
            auto* it        = DbManipulator::findTransceiver(&proxy, container);
            if (it != container.end())
            {
                // order is important - vector must be reordered with new instance id!
                container.erase(it);
                // update instance id
                proxy.setInstanceId(instanceId);
                static_cast<void>(container.emplace_back(&proxy));
                etl::sort(
                    container.begin(),
                    container.end(),
                    TransceiverContainer::TransceiverComparator());
                res = HRESULT::Ok;
            }
            else
            {
                if (container.full())
                {
                    res = HRESULT::TransceiverInitializationFailed;
                }
                else
                {
                    auto const range = getTransceiversByServiceIdAndServiceInstanceId(
                        start, end, serviceId, instanceId);
                    bool addressNotFound = true;
                    while (addressNotFound)
                    {
                        auto const* it = etl::find_if(
                            range.first,
                            range.second,
                            [&containerIt](TransceiverBase const* const itrx)
                            { return (itrx->getAddressId() == containerIt->_actualAddress); });
                        if (it == range.second)
                        {
                            addressNotFound = false;
                            proxy.setAddressId(containerIt->_actualAddress);
                            containerIt->_actualAddress++;
                        }
                        else
                        {
                            ++containerIt->_actualAddress;
                        }
                    }
                    proxy.setInstanceId(instanceId);
                    static_cast<void>(container.emplace_back(&proxy));
                    etl::sort(
                        container.begin(),
                        container.end(),
                        TransceiverContainer::TransceiverComparator());
                    res = HRESULT::Ok;
                }
            }
        }
    }
    if (HRESULT::Ok != res)
    {
        proxy.setInstanceId(INVALID_INSTANCE_ID);
    }
    return res;
}

void DbManipulator::unsubscribe(
    middleware::core::meta::TransceiverContainer* const start,
    middleware::core::meta::TransceiverContainer* const end,
    TransceiverBase& transceiver,
    uint16_t const serviceId)
{
    auto* containerIt = getTransceiversByServiceId(start, end, serviceId);
    if (containerIt != end)
    {
        auto& container  = *containerIt->_container;
        auto const range = etl::equal_range(
            container.cbegin(),
            container.cend(),
            &transceiver,
            TransceiverContainer::TransceiverComparator());
        auto const* it = etl::find_if(
            range.first,
            range.second,
            [&transceiver](TransceiverBase const* const itrx)
            { return (itrx->getAddressId() == transceiver.getAddressId()); });
        if (it != container.cend())
        {
            static_cast<void>(container.erase(it));
            transceiver.setAddressId(INVALID_ADDRESS_ID);
        }
    }
}

HRESULT
DbManipulator::subscribe(
    middleware::core::meta::TransceiverContainer* const start,
    middleware::core::meta::TransceiverContainer* const end,
    SkeletonBase& skeleton,
    uint16_t const instanceId,
    uint16_t const maxServiceId)
{
    auto res = HRESULT::ServiceNotFound;
    if (skeleton.getServiceId() > maxServiceId)
    {
        res = HRESULT::ServiceIdOutOfRange;
    }
    else
    {
        auto const serviceId = skeleton.getServiceId();
        auto* containerIt    = getTransceiversByServiceId(start, end, serviceId);
        if (containerIt != end)
        {
            auto& container = *containerIt->_container;
            auto* it        = DbManipulator::findTransceiver(&skeleton, container);
            if (it != container.end())
            {
                // order is important - vector must be reordered with new instance id!
                container.erase(it);
                // update instance id
                skeleton.setInstanceId(instanceId);
                static_cast<void>(container.emplace_back(&skeleton));
                etl::sort(
                    container.begin(),
                    container.end(),
                    TransceiverContainer::TransceiverComparator());
                res = HRESULT::InstanceAlreadyRegistered;
            }
            else
            {
                // if another skeleton with this serviceInstandId is registered fail
                if (isSkeletonWithServiceInstanceIdRegistered(container, instanceId))
                {
                    res = HRESULT::SkeletonWithThisServiceIdAlreadyRegistered;
                }
                else
                {
                    if (container.full())
                    {
                        res = HRESULT::TransceiverInitializationFailed;
                    }
                    else
                    {
                        skeleton.setInstanceId(instanceId);
                        static_cast<void>(container.emplace_back(&skeleton));
                        etl::sort(
                            container.begin(),
                            container.end(),
                            TransceiverContainer::TransceiverComparator());
                        res = HRESULT::Ok;
                    }
                }
            }
        }
    }
    if ((HRESULT::Ok != res) && (HRESULT::InstanceAlreadyRegistered != res))
    {
        skeleton.setInstanceId(INVALID_INSTANCE_ID);
    }
    return res;
}

TransceiverContainer* DbManipulator::getTransceiversByServiceId(
    middleware::core::meta::TransceiverContainer* const start,
    middleware::core::meta::TransceiverContainer* const end,
    uint16_t const serviceId)
{
    // To avoid code duplication, call const version, then cast away constness
    return const_cast<TransceiverContainer*>( // NOLINT(cppcoreguidelines-pro-type-const-cast)
        getTransceiversByServiceId(
            static_cast<middleware::core::meta::TransceiverContainer const*>(start),
            static_cast<middleware::core::meta::TransceiverContainer const*>(end),
            serviceId));
}

TransceiverContainer const* DbManipulator::getTransceiversByServiceId(
    middleware::core::meta::TransceiverContainer const* const start,
    middleware::core::meta::TransceiverContainer const* const end,
    uint16_t const serviceId)
{
    auto const* it = etl::lower_bound(
        start,
        end,
        TransceiverContainer{nullptr, serviceId, 0U},
        [](TransceiverContainer const& lhs, TransceiverContainer const& rhs) -> bool
        { return lhs._serviceId < rhs._serviceId; });
    if ((it != end) && (it->_serviceId == serviceId))
    {
        return it;
    }

    return end;
}

etl::pair<
    etl::ivector<TransceiverBase*>::const_iterator,
    etl::ivector<TransceiverBase*>::const_iterator>
DbManipulator::getTransceiversByServiceIdAndServiceInstanceId(
    middleware::core::meta::TransceiverContainer const* const start,
    middleware::core::meta::TransceiverContainer const* const end,
    uint16_t const serviceId,
    uint16_t const instanceId)
{
    auto const* transceiversById = getTransceiversByServiceId(start, end, serviceId);
    if (transceiversById != end)
    {
        internal::DummyTransceiver const dummy(instanceId);
        return etl::equal_range(
            transceiversById->_container->cbegin(),
            transceiversById->_container->cend(),
            &dummy,
            TransceiverContainer::TransceiverComparatorNoAddressId());
    }

    return etl::make_pair(start->_container->cbegin(), start->_container->cbegin());
}

TransceiverBase* DbManipulator::getSkeletonByServiceIdAndServiceInstanceId(
    middleware::core::meta::TransceiverContainer const* const start,
    middleware::core::meta::TransceiverContainer const* const end,
    uint16_t const serviceId,
    uint16_t const instanceId)
{
    auto const* transceiversById = getTransceiversByServiceId(start, end, serviceId);
    if (transceiversById != end)
    {
        internal::DummyTransceiver const dummy(instanceId);
        auto const range = etl::equal_range(
            transceiversById->_container->cbegin(),
            transceiversById->_container->cend(),
            &dummy,
            TransceiverContainer::TransceiverComparator());
        // there can be only a single skeleton with the same instanceId
        if (range.first != range.second)
        {
            return (*range.first);
        }
    }
    return nullptr;
}

etl::ivector<TransceiverBase*>::iterator DbManipulator::findTransceiver(
    TransceiverBase* const& transceiver, etl::ivector<TransceiverBase*>& container)
{
    auto* it = etl::lower_bound(
        container.begin(),
        container.end(),
        transceiver,
        TransceiverContainer::TransceiverComparator());

    if ((it != container.cend()) && (*it)->getInstanceId() == transceiver->getInstanceId()
        && (*it)->getAddressId() == transceiver->getAddressId())
    {
        return it;
    }

    return container.end();
}

bool DbManipulator::isSkeletonWithServiceInstanceIdRegistered(
    etl::ivector<TransceiverBase*> const& container, uint16_t const instanceId)
{
    internal::DummyTransceiver const dummy(instanceId);
    auto const range = etl::equal_range(
        container.cbegin(),
        container.cend(),
        &dummy,
        TransceiverContainer::TransceiverComparator());
    return (range.first != range.second);
}

TransceiverBase* DbManipulator::getTransceiver(
    middleware::core::meta::TransceiverContainer const* const start,
    middleware::core::meta::TransceiverContainer const* const end,
    uint16_t const serviceId,
    uint16_t const instanceId,
    uint16_t const addressId)
{
    auto const* containerIt = getTransceiversByServiceId(start, end, serviceId);
    if (containerIt != end)
    {
        internal::DummyTransceiver const dummy(instanceId, addressId);
        auto const* it = etl::lower_bound(
            containerIt->_container->cbegin(),
            containerIt->_container->cend(),
            &dummy,
            TransceiverContainer::TransceiverComparator());
        if ((it != containerIt->_container->cend())
            && (!TransceiverContainer::TransceiverComparator()(&dummy, *it)))
        {
            return *it;
        }
    }
    return nullptr;
}

size_t DbManipulator::registeredTransceiversCount(
    middleware::core::meta::TransceiverContainer const* const start,
    middleware::core::meta::TransceiverContainer const* const end,
    uint16_t const serviceId)
{
    auto const* containerIt = getTransceiversByServiceId(start, end, serviceId);
    if (containerIt != end)
    {
        return containerIt->_container->size();
    }
    return 0U;
}

} // namespace middleware::core::meta
