// Copyright 2025 BMW AG

#include "middleware/core/DatabaseManipulator.h"

#include <cstddef>
#include <cstdint>

#include <etl/algorithm.h>
#include <etl/utility.h>
#include <etl/vector.h>

#include "middleware/concurrency/LockStrategies.h"
#include "middleware/core/ITransceiver.h"
#include "middleware/core/ProxyBase.h"
#include "middleware/core/SkeletonBase.h"
#include "middleware/core/TransceiverContainer.h"
#include "middleware/core/types.h"

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
        auto const serviceId    = proxy.getServiceId();
        auto* containerIterator = getTransceiversByServiceId(start, end, serviceId);
        if (containerIterator != end)
        {
            auto* iter = DbManipulator::findTransceiver(&proxy, *containerIterator->fContainer);
            if (iter != containerIterator->fContainer->end())
            {
                // order is important - vector must be reordered with new instance id!
                containerIterator->fContainer->erase(iter);
                // update instance id
                proxy.setInstanceId(instanceId);
                static_cast<void>(containerIterator->fContainer->emplace_back(&proxy));
                etl::sort(
                    containerIterator->fContainer->begin(),
                    containerIterator->fContainer->end(),
                    TransceiverContainer::TransceiverComparator());
                res = HRESULT::Ok;
            }
            else
            {
                if (containerIterator->fContainer->full())
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
                        auto const* const iter = etl::find_if(
                            range.first,
                            range.second,
                            [&containerIterator](ITransceiver const* const itrx) {
                                return (itrx->getAddressId() == containerIterator->fActualAddress);
                            });
                        if (iter == range.second)
                        {
                            addressNotFound = false;
                            proxy.setAddressId(containerIterator->fActualAddress);
                            containerIterator->fActualAddress++;
                        }
                        else
                        {
                            ++containerIterator->fActualAddress;
                        }
                    }
                    proxy.setInstanceId(instanceId);
                    static_cast<void>(containerIterator->fContainer->emplace_back(&proxy));
                    etl::sort(
                        containerIterator->fContainer->begin(),
                        containerIterator->fContainer->end(),
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
    ITransceiver& transceiver,
    uint16_t const serviceId)
{
    auto* const containerIterator = getTransceiversByServiceId(start, end, serviceId);
    if (containerIterator != end)
    {
        auto const range = etl::equal_range(
            containerIterator->fContainer->cbegin(),
            containerIterator->fContainer->cend(),
            &transceiver,
            TransceiverContainer::TransceiverComparator());
        auto const* const iter = etl::find_if(
            range.first,
            range.second,
            [&transceiver](ITransceiver const* const itrx)
            { return (itrx->getAddressId() == transceiver.getAddressId()); });
        if (iter != containerIterator->fContainer->cend())
        {
            static_cast<void>(containerIterator->fContainer->erase(iter));
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
        auto const serviceId          = skeleton.getServiceId();
        auto* const containerIterator = getTransceiversByServiceId(start, end, serviceId);
        if (containerIterator != end)
        {
            auto* iter = DbManipulator::findTransceiver(&skeleton, *containerIterator->fContainer);
            if (iter != containerIterator->fContainer->end())
            {
                // order is important - vector must be reordered with new instance id!
                containerIterator->fContainer->erase(iter);
                // update instance id
                skeleton.setInstanceId(instanceId);
                static_cast<void>(containerIterator->fContainer->emplace_back(&skeleton));
                etl::sort(
                    containerIterator->fContainer->begin(),
                    containerIterator->fContainer->end(),
                    TransceiverContainer::TransceiverComparator());
                res = HRESULT::InstanceAlreadyRegistered;
            }
            else
            {
                // if another skeleton with this serviceInstandId is registered fail
                if (isSkeletonWithServiceInstanceIdRegistered(
                        *containerIterator->fContainer, instanceId))
                {
                    res = HRESULT::SkeletonWithThisServiceIdAlreadyRegistered;
                }
                else
                {
                    if (containerIterator->fContainer->full())
                    {
                        res = HRESULT::TransceiverInitializationFailed;
                    }
                    else
                    {
                        skeleton.setInstanceId(instanceId);
                        static_cast<void>(containerIterator->fContainer->emplace_back(&skeleton));
                        etl::sort(
                            containerIterator->fContainer->begin(),
                            containerIterator->fContainer->end(),
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
    auto const* const iter = etl::lower_bound(
        start,
        end,
        TransceiverContainer{nullptr, serviceId, 0U},
        [](TransceiverContainer const& lhs, TransceiverContainer const& rhs) -> bool
        { return lhs.fServiceid < rhs.fServiceid; });
    if ((iter != end) && (iter->fServiceid == serviceId))
    {
        return iter;
    }

    return end;
}

etl::pair<etl::ivector<ITransceiver*>::const_iterator, etl::ivector<ITransceiver*>::const_iterator>
DbManipulator::getTransceiversByServiceIdAndServiceInstanceId(
    middleware::core::meta::TransceiverContainer const* const start,
    middleware::core::meta::TransceiverContainer const* const end,
    uint16_t const serviceId,
    uint16_t const instanceId)
{
    auto const* const transceiversById = getTransceiversByServiceId(start, end, serviceId);
    if (transceiversById != end)
    {
        internal::DummyTransceiver const dummy(instanceId);
        return etl::equal_range(
            transceiversById->fContainer->cbegin(),
            transceiversById->fContainer->cend(),
            &dummy,
            TransceiverContainer::TransceiverComparatorNoAddressId());
    }

    return etl::make_pair(start->fContainer->cbegin(), start->fContainer->cbegin());
}

ITransceiver* DbManipulator::getSkeletonByServiceIdAndServiceInstanceId(
    middleware::core::meta::TransceiverContainer const* const start,
    middleware::core::meta::TransceiverContainer const* const end,
    uint16_t const serviceId,
    uint16_t const instanceId)
{
    auto const* const transceiversById = getTransceiversByServiceId(start, end, serviceId);
    if (transceiversById != end)
    {
        internal::DummyTransceiver const dummy(instanceId);
        auto const range = etl::equal_range(
            transceiversById->fContainer->cbegin(),
            transceiversById->fContainer->cend(),
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

etl::ivector<ITransceiver*>::iterator DbManipulator::findTransceiver(
    ITransceiver* const& transceiver, etl::ivector<ITransceiver*>& container)
{
    auto* const iter = etl::lower_bound(
        container.begin(),
        container.end(),
        transceiver,
        TransceiverContainer::TransceiverComparator());

    if ((iter != container.cend()) && (*iter)->getInstanceId() == transceiver->getInstanceId()
        && (*iter)->getAddressId() == transceiver->getAddressId())
    {
        return iter;
    }

    return container.end();
}

bool DbManipulator::isSkeletonWithServiceInstanceIdRegistered(
    etl::ivector<ITransceiver*> const& container, uint16_t const instanceId)
{
    internal::DummyTransceiver const dummy(instanceId);
    auto const range = etl::equal_range(
        container.cbegin(),
        container.cend(),
        &dummy,
        TransceiverContainer::TransceiverComparator());
    return (range.first != range.second);
}

ITransceiver* DbManipulator::getTransceiver(
    middleware::core::meta::TransceiverContainer const* const start,
    middleware::core::meta::TransceiverContainer const* const end,
    uint16_t const serviceId,
    uint16_t const instanceId,
    uint16_t const addressId)
{
    auto const* const containerIterator = getTransceiversByServiceId(start, end, serviceId);
    if (containerIterator != end)
    {
        internal::DummyTransceiver const dummy(instanceId, addressId);
        auto const* const iter = etl::lower_bound(
            containerIterator->fContainer->cbegin(),
            containerIterator->fContainer->cend(),
            &dummy,
            TransceiverContainer::TransceiverComparator());
        if ((iter != containerIterator->fContainer->cend())
            && (!TransceiverContainer::TransceiverComparator()(&dummy, *iter)))
        {
            return *iter;
        }
    }
    return nullptr;
}

size_t DbManipulator::registeredTransceiversCount(
    middleware::core::meta::TransceiverContainer const* const start,
    middleware::core::meta::TransceiverContainer const* const end,
    uint16_t const serviceId)
{
    auto const* const containerIterator = getTransceiversByServiceId(start, end, serviceId);
    if (containerIterator != end)
    {
        return containerIterator->fContainer->size();
    }
    return 0U;
}

} // namespace middleware::core::meta
