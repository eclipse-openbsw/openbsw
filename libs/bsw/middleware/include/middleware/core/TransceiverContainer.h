// Copyright 2025 BMW AG

#pragma once

#include "middleware/core/TransceiverBase.h"
#include "middleware/core/types.h"

#include <etl/utility.h>
#include <etl/vector.h>

#include <cstdint>

namespace middleware::core::meta
{
/**
 * Container for managing transceiver collections.
 * This struct holds a collection of transceivers associated with a specific service ID.
 * It provides comparator functors for sorting and searching transceivers by instance ID and
 * address ID. The container is used by the middleware database to organize and manage proxies
 * and skeletons.
 */
struct TransceiverContainer
{
    /**
     * Comparator for transceivers with instance ID and address ID.
     * Compares transceivers based on a pair of (instance ID, address ID), providing
     * a total ordering for transceivers. This is used when both instance and address
     * identification are needed.
     */
    struct TransceiverComparator
    {
        /**
         * Compare two transceivers by instance ID and address ID.
         * Returns true if the left transceiver's (instanceId, addressId) pair is less
         * than the right transceiver's pair using lexicographical comparison.
         *
         * \param lhs const pointer to the left-hand transceiver
         * \param rhs const pointer to the right-hand transceiver
         * \return true if lhs < rhs, false otherwise
         */
        bool operator()(TransceiverBase const* const lhs, TransceiverBase const* const rhs) const
        {
            return (
                etl::make_pair(lhs->getInstanceId(), lhs->getAddressId())
                < etl::make_pair(rhs->getInstanceId(), rhs->getAddressId()));
        }
    };

    /**
     * Comparator for transceivers by instance ID only.
     * Compares transceivers based solely on their instance ID, ignoring address ID.
     * This is used when only instance-level identification is needed, such as when searching
     * for skeletons.
     */
    struct TransceiverComparatorNoAddressId
    {
        /**
         * Compare two transceivers by instance ID only.
         * Returns true if the left transceiver's instance ID is less than the right
         * transceiver's instance ID.
         *
         * \param lhs const pointer to the left-hand transceiver
         * \param rhs const pointer to the right-hand transceiver
         * \return true if lhs instance ID < rhs instance ID, false otherwise
         */
        bool operator()(TransceiverBase const* const lhs, TransceiverBase const* const rhs) const
        {
            return (lhs->getInstanceId() < rhs->getInstanceId());
        }
    };

    TransceiverContainer()  = delete;
    ~TransceiverContainer() = default;

    constexpr TransceiverContainer(
        etl::ivector<TransceiverBase*>* container, uint16_t serviceid, uint16_t actualAddress)
    : _container(container), _serviceId(serviceid), _actualAddress(actualAddress)
    {}

    TransceiverContainer(TransceiverContainer const&)            = delete;
    TransceiverContainer& operator=(TransceiverContainer const&) = delete;
    TransceiverContainer(TransceiverContainer&&)                 = delete;
    TransceiverContainer& operator=(TransceiverContainer&&)      = delete;

    /** Pointer to the vector holding transceiver pointers. */
    etl::ivector<TransceiverBase*>* const _container{};
    /** The service ID associated with this container. */
    uint16_t const _serviceId;
    /** The current/next available address ID for proxies. */
    uint16_t _actualAddress;
};

} // namespace middleware::core::meta
