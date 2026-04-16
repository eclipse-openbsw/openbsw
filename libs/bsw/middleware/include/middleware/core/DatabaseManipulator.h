// Copyright 2025 BMW AG

#pragma once

#include "middleware/core/TransceiverBase.h"
#include "middleware/core/TransceiverContainer.h"
#include "middleware/core/types.h"

#include <etl/utility.h>
#include <etl/vector.h>

#include <cstdint>

namespace middleware::core
{
class ProxyBase;
class SkeletonBase;

namespace meta
{

namespace internal
{
/**
 * Dummy transceiver implementation for testing and placeholder purposes.
 * This transceiver provides minimal functionality and is typically used as a
 * placeholder in the transceiver database. It returns default values for most operations
 * and does not perform actual message processing.
 */
struct DummyTransceiver final : public TransceiverBase
{
    using Base = TransceiverBase;

    /** No-op: always returns HRESULT::Ok. */
    HRESULT onNewMessageReceived(Message const&) override { return HRESULT::Ok; }

    /** Returns INVALID_SERVICE_ID. */
    uint16_t getServiceId() const override { return INVALID_SERVICE_ID; }

    /** Returns 0xFF. */
    uint8_t getSourceClusterId() const override { return static_cast<uint8_t>(0xFFU); }

    /** Returns false always. */
    bool isInitialized() const override { return false; }

    /** No-op: always returns HRESULT::Ok. */
    HRESULT sendMessage(Message&) const override { return HRESULT::Ok; }

    /** \see TransceiverBase::getAddressId() */
    uint8_t getAddressId() const override { return _addressId; }

    /** \see TransceiverBase::setAddressId() */
    void setAddressId(uint8_t const addressId) override { _addressId = addressId; }

    /** Constructs with \p instanceId and optional \p addressId. */
    explicit DummyTransceiver(
        uint16_t const instanceId, uint8_t const addressId = INVALID_ADDRESS_ID)
    : Base(instanceId), _addressId(addressId)
    {}

    ~DummyTransceiver() = default;

private:
    /** The address ID of the dummy transceiver. */
    uint8_t _addressId;
};

} // namespace internal

/**
 * Database manipulator for managing transceiver subscriptions and lookups.
 * This class provides static utility methods for managing the transceiver database,
 * including subscription management, transceiver lookups, and database queries. It operates on
 * ranges of TransceiverContainer objects and provides efficient access to transceivers by
 * service ID, instance ID, and address ID.
 */
class DbManipulator
{
public:
    /**
     * Subscribe a proxy to the transceiver database.
     * Registers the proxy with the specified instance ID in the transceiver database,
     * making it available to receive messages for the associated service.
     *
     * \param start pointer to the start of the transceiver container range
     * \param end pointer to the end of the transceiver container range
     * \param proxy reference to the proxy to subscribe
     * \param instanceId the service instance ID for the subscription
     * \param maxServiceId the maximum service ID in the database
     * \return HRESULT indicating success or failure of the subscription
     */
    static HRESULT subscribe(
        middleware::core::meta::TransceiverContainer* const start,
        middleware::core::meta::TransceiverContainer* const end,
        ProxyBase& proxy,
        uint16_t instanceId,
        uint16_t maxServiceId);

    /**
     * Subscribe a skeleton to the transceiver database.
     * Registers the skeleton with the specified instance ID in the transceiver database,
     * making it available to receive messages for the associated service.
     *
     * \param start pointer to the start of the transceiver container range
     * \param end pointer to the end of the transceiver container range
     * \param skeleton reference to the skeleton to subscribe
     * \param instanceId the service instance ID for the subscription
     * \param maxServiceId the maximum service ID in the database
     * \return HRESULT indicating success or failure of the subscription
     */
    static HRESULT subscribe(
        middleware::core::meta::TransceiverContainer* const start,
        middleware::core::meta::TransceiverContainer* const end,
        SkeletonBase& skeleton,
        uint16_t instanceId,
        uint16_t maxServiceId);

    /**
     * Unsubscribe a transceiver from the database.
     * Removes the transceiver with the specified service ID from the transceiver
     * database, stopping it from receiving further messages.
     *
     * \param start pointer to the start of the transceiver container range
     * \param end pointer to the end of the transceiver container range
     * \param transceiver reference to the transceiver to unsubscribe
     * \param serviceId the service ID for the unsubscription
     */
    static void unsubscribe(
        middleware::core::meta::TransceiverContainer* const start,
        middleware::core::meta::TransceiverContainer* const end,
        TransceiverBase& transceiver,
        uint16_t serviceId);

    /** Returns a pointer to the transceiver container for \p serviceId, or nullptr if not found. */
    static TransceiverContainer* getTransceiversByServiceId(
        middleware::core::meta::TransceiverContainer* const start,
        middleware::core::meta::TransceiverContainer* const end,
        uint16_t serviceId);

    /** Returns a const pointer to the transceiver container for \p serviceId, or nullptr if not
     * found. */
    static TransceiverContainer const* getTransceiversByServiceId(
        middleware::core::meta::TransceiverContainer const* const start,
        middleware::core::meta::TransceiverContainer const* const end,
        uint16_t serviceId);

    /**
     * Get transceivers by service ID and service instance ID.
     * Returns an iterator pair representing the range of transceivers that match both
     * the service ID and service instance ID.
     *
     * \param start const pointer to the start of the transceiver container range
     * \param end const pointer to the end of the transceiver container range
     * \param serviceId the service ID to query
     * \param instanceId the service instance ID to query
     * \return pair of const iterators representing the begin and end of the matching range
     */
    static etl::pair<
        etl::ivector<TransceiverBase*>::const_iterator,
        etl::ivector<TransceiverBase*>::const_iterator>
    getTransceiversByServiceIdAndServiceInstanceId(
        middleware::core::meta::TransceiverContainer const* const start,
        middleware::core::meta::TransceiverContainer const* const end,
        uint16_t serviceId,
        uint16_t instanceId);

    /** Returns the skeleton matching \p serviceId and \p instanceId, or nullptr if not found. */
    static TransceiverBase* getSkeletonByServiceIdAndServiceInstanceId(
        middleware::core::meta::TransceiverContainer const* const start,
        middleware::core::meta::TransceiverContainer const* const end,
        uint16_t serviceId,
        uint16_t instanceId);

    /** Returns an iterator to \p transceiver in \p container, or end() if not found. */
    static etl::ivector<TransceiverBase*>::iterator
    findTransceiver(TransceiverBase* const& transceiver, etl::ivector<TransceiverBase*>& container);

    /** Returns true if a skeleton with \p instanceId is registered in \p container. */
    static bool isSkeletonWithServiceInstanceIdRegistered(
        etl::ivector<TransceiverBase*> const& container, uint16_t instanceId);

    /**
     * Get a transceiver by service ID, instance ID, and address ID.
     * Returns a pointer to the transceiver that matches all three identifiers: service
     * ID, service instance ID, and address ID.
     *
     * \param start const pointer to the start of the transceiver container range
     * \param end const pointer to the end of the transceiver container range
     * \param serviceId the service ID to query
     * \param instanceId the service instance ID to query
     * \param addressId the address ID to query
     * \return pointer to the matching transceiver, or nullptr if not found
     */
    static TransceiverBase* getTransceiver(
        middleware::core::meta::TransceiverContainer const* const start,
        middleware::core::meta::TransceiverContainer const* const end,
        uint16_t serviceId,
        uint16_t instanceId,
        uint16_t addressId);

    /** Returns count of registered transceivers for \p serviceId. */
    static size_t registeredTransceiversCount(
        middleware::core::meta::TransceiverContainer const* const start,
        middleware::core::meta::TransceiverContainer const* const end,
        uint16_t serviceId);
};
} // namespace meta
} // namespace middleware::core
