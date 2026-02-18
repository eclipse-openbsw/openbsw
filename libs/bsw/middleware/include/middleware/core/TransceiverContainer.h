// Copyright 2025 BMW AG

#pragma once

#include <cstdint>

#include <etl/utility.h>
#include <etl/vector.h>

#include "middleware/core/ITransceiver.h"
#include "middleware/core/types.h"

namespace middleware
{
namespace core
{
namespace meta
{
/**
 * \brief Container for managing transceiver collections.
 * \details This struct holds a collection of transceivers associated with a specific service ID.
 * It provides comparator functors for sorting and searching transceivers by instance ID and
 * address ID. The container is used by the middleware database to organize and manage proxies
 * and skeletons.
 */
struct TransceiverContainer
{
    /**
     * \brief Comparator for transceivers with instance ID and address ID.
     * \details Compares transceivers based on a pair of (instance ID, address ID), providing
     * a total ordering for transceivers. This is used when both instance and address
     * identification are needed.
     */
    struct TransceiverComparator
    {
        /**
         * \brief Compare two transceivers by instance ID and address ID.
         * \details Returns true if the left transceiver's (instanceId, addressId) pair is less
         * than the right transceiver's pair using lexicographical comparison.
         *
         * \param lhs const pointer to the left-hand transceiver
         * \param rhs const pointer to the right-hand transceiver
         * \return true if lhs < rhs, false otherwise
         */
        inline bool operator()(ITransceiver const* const lhs, ITransceiver const* const rhs) const
        {
            return (
                etl::make_pair(lhs->getInstanceId(), lhs->getAddressId())
                < etl::make_pair(rhs->getInstanceId(), rhs->getAddressId()));
        }
    };

    /**
     * \brief Comparator for transceivers by instance ID only.
     * \details Compares transceivers based solely on their instance ID, ignoring address ID.
     * This is used when only instance-level identification is needed, such as when searching
     * for skeletons.
     */
    struct TransceiverComparatorNoAddressId
    {
        /**
         * \brief Compare two transceivers by instance ID only.
         * \details Returns true if the left transceiver's instance ID is less than the right
         * transceiver's instance ID.
         *
         * \param lhs const pointer to the left-hand transceiver
         * \param rhs const pointer to the right-hand transceiver
         * \return true if lhs instance ID < rhs instance ID, false otherwise
         */
        inline bool operator()(ITransceiver const* const lhs, ITransceiver const* const rhs) const
        {
            return (lhs->getInstanceId() < rhs->getInstanceId());
        }
    };

    TransceiverContainer()  = delete;
    ~TransceiverContainer() = default;

    constexpr TransceiverContainer(
        etl::ivector<ITransceiver*>* container, uint16_t serviceid, uint16_t actualAddress)
    : fContainer(container), fServiceid(serviceid), fActualAddress(actualAddress)
    {}

    TransceiverContainer(TransceiverContainer const&)            = delete;
    TransceiverContainer& operator=(TransceiverContainer const&) = delete;
    TransceiverContainer(TransceiverContainer&&)                 = delete;
    TransceiverContainer& operator=(TransceiverContainer&&)      = delete;

    etl::ivector<ITransceiver*>* const
        fContainer{};          ///< Pointer to the vector holding transceiver pointers
    uint16_t const fServiceid; ///< The service ID associated with this container
    uint16_t fActualAddress;   ///< The current/next available address ID for proxies
};

} // namespace meta
} // namespace core
} // namespace middleware
