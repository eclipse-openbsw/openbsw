// Copyright 2025 BMW AG

#pragma once

#include <cstdint>

#include <etl/utility.h>
#include <etl/vector.h>

#include "middleware/core/ITransceiver.h"
#include "middleware/core/TransceiverContainer.h"
#include "middleware/core/types.h"

namespace middleware
{
namespace core
{
class ProxyBase;
class SkeletonBase;

namespace meta
{

namespace internal
{
/**
 * \brief Dummy transceiver implementation for testing and placeholder purposes.
 * \details This transceiver provides minimal functionality and is typically used as a
 * placeholder in the transceiver database. It returns default values for most operations
 * and does not perform actual message processing.
 */
struct DummyTransceiver final : public ITransceiver
{
    using Base = ITransceiver;

    /**
     * \brief Handle reception of a new message (no-op).
     * \param msg constant reference to the received message
     * \return HRESULT::Ok always
     */
    virtual HRESULT onNewMessageReceived(Message const&) override { return HRESULT::Ok; }

    /**
     * \brief Get the service ID.
     * \return INVALID_SERVICE_ID
     */
    virtual uint16_t getServiceId() const override { return INVALID_SERVICE_ID; }

    /**
     * \brief Get the source cluster ID.
     * \return 0xFF
     */
    virtual uint8_t getSourceClusterId() const override { return static_cast<uint8_t>(0xFFU); }

    /**
     * \brief Check if the transceiver is initialized.
     * \return false always
     */
    virtual bool isInitialized() const override { return false; }

    /**
     * \brief Send a message (no-op).
     * \return HRESULT::Ok always
     */
    virtual HRESULT sendMessage(Message&) const override { return HRESULT::Ok; }

    /**
     * \brief Get the address ID of the transceiver.
     * \return the address ID
     */
    virtual uint8_t getAddressId() const override { return fAddressId; }

    /**
     * \brief Set the address ID of the transceiver.
     * \param addressId the new address ID to set
     */
    virtual void setAddressId(uint8_t const addressId) override { fAddressId = addressId; }

    /**
     * \brief Constructor for DummyTransceiver.
     * \param instanceId the service instance ID
     * \param addressId the address ID (defaults to INVALID_ADDRESS_ID)
     */
    explicit DummyTransceiver(
        uint16_t const instanceId, uint8_t const addressId = INVALID_ADDRESS_ID)
    : Base(instanceId), fAddressId(addressId)
    {}

    virtual ~DummyTransceiver() = default;

private:
    uint8_t fAddressId; ///< The address ID of the dummy transceiver
};

} // namespace internal

/**
 * \brief Database manipulator for managing transceiver subscriptions and lookups.
 * \details This class provides static utility methods for managing the transceiver database,
 * including subscription management, transceiver lookups, and database queries. It operates on
 * ranges of TransceiverContainer objects and provides efficient access to transceivers by
 * service ID, instance ID, and address ID.
 */
class DbManipulator
{
public:
    /**
     * \brief Subscribe a proxy to the transceiver database.
     * \details Registers the proxy with the specified instance ID in the transceiver database,
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
        uint16_t const instanceId,
        uint16_t const maxServiceId);

    /**
     * \brief Subscribe a skeleton to the transceiver database.
     * \details Registers the skeleton with the specified instance ID in the transceiver database,
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
        uint16_t const instanceId,
        uint16_t const maxServiceId);

    /**
     * \brief Unsubscribe a transceiver from the database.
     * \details Removes the transceiver with the specified service ID from the transceiver
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
        ITransceiver& transceiver,
        uint16_t const serviceId);

    /**
     * \brief Get transceivers by service ID (mutable version).
     * \details Returns a pointer to the transceiver container for the specified service ID,
     * allowing modifications.
     *
     * \param start pointer to the start of the transceiver container range
     * \param end pointer to the end of the transceiver container range
     * \param serviceId the service ID to query
     * \return pointer to the transceiver container, or nullptr if not found
     */
    static TransceiverContainer* getTransceiversByServiceId(
        middleware::core::meta::TransceiverContainer* const start,
        middleware::core::meta::TransceiverContainer* const end,
        uint16_t const serviceId);

    /**
     * \brief Get transceivers by service ID (const version).
     * \details Returns a const pointer to the transceiver container for the specified service ID.
     *
     * \param start const pointer to the start of the transceiver container range
     * \param end const pointer to the end of the transceiver container range
     * \param serviceId the service ID to query
     * \return const pointer to the transceiver container, or nullptr if not found
     */
    static TransceiverContainer const* getTransceiversByServiceId(
        middleware::core::meta::TransceiverContainer const* const start,
        middleware::core::meta::TransceiverContainer const* const end,
        uint16_t const serviceId);

    /**
     * \brief Get transceivers by service ID and service instance ID.
     * \details Returns an iterator pair representing the range of transceivers that match both
     * the service ID and service instance ID.
     *
     * \param start const pointer to the start of the transceiver container range
     * \param end const pointer to the end of the transceiver container range
     * \param serviceId the service ID to query
     * \param instanceId the service instance ID to query
     * \return pair of const iterators representing the begin and end of the matching range
     */
    static etl::pair<
        etl::ivector<ITransceiver*>::const_iterator,
        etl::ivector<ITransceiver*>::const_iterator>
    getTransceiversByServiceIdAndServiceInstanceId(
        middleware::core::meta::TransceiverContainer const* const start,
        middleware::core::meta::TransceiverContainer const* const end,
        uint16_t const serviceId,
        uint16_t const instanceId);

    /**
     * \brief Get a skeleton by service ID and service instance ID.
     * \details Returns a pointer to the skeleton transceiver that matches both the service ID
     * and service instance ID.
     *
     * \param start const pointer to the start of the transceiver container range
     * \param end const pointer to the end of the transceiver container range
     * \param serviceId the service ID to query
     * \param instanceId the service instance ID to query
     * \return pointer to the skeleton transceiver, or nullptr if not found
     */
    static ITransceiver* getSkeletonByServiceIdAndServiceInstanceId(
        middleware::core::meta::TransceiverContainer const* const start,
        middleware::core::meta::TransceiverContainer const* const end,
        uint16_t const serviceId,
        uint16_t const instanceId);

    /**
     * \brief Find a transceiver in a container.
     * \details Searches for the specified transceiver in the given container and returns an
     * iterator to its position.
     *
     * \param transceiver pointer to the transceiver to find
     * \param container reference to the vector of transceiver pointers to search
     * \return iterator to the found transceiver, or end() if not found
     */
    static etl::ivector<ITransceiver*>::iterator
    findTransceiver(ITransceiver* const& transceiver, etl::ivector<ITransceiver*>& container);

    /**
     * \brief Check if a skeleton with the given instance ID is registered.
     * \details Checks whether a skeleton transceiver with the specified service instance ID
     * exists in the container.
     *
     * \param container const reference to the vector of transceiver pointers to check
     * \param instanceId the service instance ID to check for
     * \return true if a skeleton with the instance ID is registered, false otherwise
     */
    static bool isSkeletonWithServiceInstanceIdRegistered(
        etl::ivector<ITransceiver*> const& container, uint16_t const instanceId);

    /**
     * \brief Get a transceiver by service ID, instance ID, and address ID.
     * \details Returns a pointer to the transceiver that matches all three identifiers: service
     * ID, service instance ID, and address ID.
     *
     * \param start const pointer to the start of the transceiver container range
     * \param end const pointer to the end of the transceiver container range
     * \param serviceId the service ID to query
     * \param instanceId the service instance ID to query
     * \param addressId the address ID to query
     * \return pointer to the matching transceiver, or nullptr if not found
     */
    static ITransceiver* getTransceiver(
        middleware::core::meta::TransceiverContainer const* const start,
        middleware::core::meta::TransceiverContainer const* const end,
        uint16_t const serviceId,
        uint16_t const instanceId,
        uint16_t const addressId);

    /**
     * \brief Get the count of registered transceivers for a service.
     * \details Returns the total number of transceivers registered for the specified service ID.
     *
     * \param start const pointer to the start of the transceiver container range
     * \param end const pointer to the end of the transceiver container range
     * \param serviceId the service ID to query
     * \return the number of registered transceivers
     */
    static size_t registeredTransceiversCount(
        middleware::core::meta::TransceiverContainer const* const start,
        middleware::core::meta::TransceiverContainer const* const end,
        uint16_t const serviceId);
};
} // namespace meta
} // namespace core
} // namespace middleware
