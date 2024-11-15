// Copyright 2024 Accenture.

#pragma once

#include "platform/estdint.h"
#include "transport/TransportJob.h"
#include "uds/UdsConfig.h"
#include "uds/connection/IncomingDiagConnection.h"
#include "uds/connection/ManagedOutgoingDiagConnection.h"

#include <etl/algorithm.h>
#include <etl/intrusive_list.h>
#include <etl/pool.h>
#include <etl/queue.h>
#include <etl/utility.h>

namespace uds
{
/**
 * Interface to the DiagnosisConfiguration used by client classes.
 *
 * This class does only use container classes that are independent of a
 * static size. So any class using this configuration should be implemented
 * that way, e.g. use size() and max_size() functions of container classes.
 */
class AbstractDiagnosisConfiguration
{
public:
    using ManagedIncomingDiagConnectionPool = ::etl::ipool;
    using ManagedOutgoingDiagConnectionList
        = ::etl::intrusive_list<ManagedOutgoingDiagConnection, ::etl::bidirectional_link<0>>;
    using TransportJobQueue = ::etl::iqueue<transport::TransportJob>;

    /**
     * Constructor
     * @param   diagAddress Diagnosis address of this configuration. Used e.g.
     * by DiagDispatcher2 to filter incoming requests.
     * @param   broadcastAddress    In addition to the specific diagnosis address
     * a diagnosis layer may also receive requests that are broadcasted within
     * the system.
     * @param   busId   The diagnosis layers BusId.
     * @param   activateOutgoingPending Flag to enable response pending responses
     * being sent automatically by an IncomingDiagConnection.
     * @param   copyFunctionalRequests  Copy functional requests before processing.
     * To keep the default behaviour use 'true' here, otherwise it is up to the
     * application to provide only maximum sized messages that may be modified to
     * the dispatcher.
     */
    AbstractDiagnosisConfiguration(
        uint16_t const diagAddress,
        uint16_t const broadcastAddress,
        uint8_t const busId,
        uint16_t const maxResponsePayloadSize,
        bool const activateOutgoingPending,
        bool const acceptAllRequests,
        bool const copyFunctionalRequests,
        ManagedIncomingDiagConnectionPool& incomingPool,
        TransportJobQueue& sendJobQueue,
        ::async::ContextType const context)
    : IncomingDiagConnectionPool(incomingPool)
    , OutgoingDiagConnections()
    , SendJobQueue(sendJobQueue)
    , DiagAddress(diagAddress)
    , BroadcastAddress(broadcastAddress)
    , MaxResponsePayloadSize(maxResponsePayloadSize)
    , DiagBusId(busId)
    , ActivateOutgoingPending(activateOutgoingPending)
    , AcceptAllRequests(acceptAllRequests)
    , CopyFunctionalRequests(copyFunctionalRequests)
    , Context(context)
    {}

private:
    ManagedIncomingDiagConnectionPool& IncomingDiagConnectionPool;

public:
    ManagedOutgoingDiagConnectionList OutgoingDiagConnections;
    TransportJobQueue& SendJobQueue;

#ifdef IS_VARIANT_HANDLING_NEEDED
    uint16_t DiagAddress;
#else
    uint16_t const DiagAddress;
#endif

    uint16_t const BroadcastAddress;
    uint16_t const MaxResponsePayloadSize;
    uint8_t DiagBusId;
    bool ActivateOutgoingPending;
    bool AcceptAllRequests;
    bool CopyFunctionalRequests;
    ::async::ContextType Context;

    /**
     * Returns a read only reference to IncomingDiagConnectionPool.
     *
     * This can be useful to do statistics by reading the current size of the pool.
     */
    ManagedIncomingDiagConnectionPool const& incomingDiagConnectionPool() const
    {
        return IncomingDiagConnectionPool;
    }

    /**
     * Returns an acquired IncomingDiagConnection initialized with the appropriate async
     * context, nullptr if no connection is available.
     */
    IncomingDiagConnection* acquireIncomingDiagConnection()
    {
        if (IncomingDiagConnectionPool.full())
        {
            return nullptr;
        }
        return IncomingDiagConnectionPool
            .template create<IncomingDiagConnection, ::async::ContextType const>(
                etl::move(Context));
    }

    /**
     * Releases a given IncomingDiagConnection back to the pool.
     */
    void releaseIncomingDiagConnection(IncomingDiagConnection const& connection)
    {
        IncomingDiagConnectionPool.release(&connection);
    }

    template<class Pred>
    IncomingDiagConnection* findIncomingDiagConnection(Pred const condition)
    {
        auto result = etl::find_if(
            IncomingDiagConnectionPool.begin(), IncomingDiagConnectionPool.end(), condition);

        if (result == IncomingDiagConnectionPool.end())
        {
            return nullptr;
        }

        return &result.template get<IncomingDiagConnection>();
    }

    /**
     * Clears the pool of ManagedIncomingDiagConnections (destroying all released elements).
     * This function should only be called in case of an error and might have unforeseen
     * effects because some user code might still hold a reference to an elements which
     * gets destroyed when this function is called.
     */
    void clearIncomingDiagConnections() { IncomingDiagConnectionPool.release_all(); }

protected:
    void init(
        ManagedOutgoingDiagConnection* const outgoingConnections,
        TransportJobQueue** const responseQueues,
        uint8_t const numberOfOutgoingConnections)
    {
        for (uint8_t i = 0U; i < numberOfOutgoingConnections; ++i)
        {
            outgoingConnections[i].setResponseQueue(*responseQueues[i]);
            OutgoingDiagConnections.push_back(outgoingConnections[i]);
        }
    }
};

/**
 * Configuration for a diagnosis layer
 * @tparam  NUM_INCOMING_CONNECTIONS    number of incoming connections that are
 * handled by the layer (i.e. simultaneous requests from outside to the layer)
 * @tparam  NUM_OUTGOING_CONNECTIONS    number of outgoing connections provided
 * by the layer (i.e. number of simultaneous request that applications may
 * use to send requests
 * @tparam  MAX_NUM_INCOMING_MESSAGES   this parameter specifies the maximum
 * number of TransportMessages that can be queued inside the diagnosis
 * layer. For a functional request this number should equal the number of
 * TransportMessages available in the system.
 */
template<
    uint8_t NUM_INCOMING_CONNECTIONS,
    uint8_t NUM_OUTGOING_CONNECTIONS,
    uint8_t MAX_NUM_INCOMING_MESSAGES>
class DiagnosisConfiguration : public AbstractDiagnosisConfiguration
{
public:
    /**
     * Constructor
     */
    explicit DiagnosisConfiguration(
        uint16_t const diagAddress,
        uint16_t const broadcastAddress,
        uint8_t const busId,
        uint16_t const maxResponsePayloadSize,
        bool const activateOutgoingPending,
        bool const acceptAllRequests,
        bool const copyFunctionalRequests,
        ::async::ContextType const context)
    : AbstractDiagnosisConfiguration(
        diagAddress,
        broadcastAddress,
        busId,
        maxResponsePayloadSize,
        activateOutgoingPending,
        acceptAllRequests,
        copyFunctionalRequests,
        fIncomingDiagConnectionPool,
        fSendJobQueue,
        context)
    , fIncomingDiagConnectionPool()
    , fOutgoingDiagConnections()
    , fSendJobQueue()
    , fResponseQueues()
    {
        // Because we must not use fResponseQueues polymorphical, i.e. passing
        // it as an array of ::etl::iqueue to AbstractDiagnosisConfiguration
        // we need to convert it first
        ::etl::iqueue<transport::TransportJob>* baseQueues[NUM_OUTGOING_CONNECTIONS];
        for (uint8_t i = 0U; i < NUM_OUTGOING_CONNECTIONS; ++i)
        {
            baseQueues[i] = &fResponseQueues[i];
        }
        AbstractDiagnosisConfiguration::init(
            fOutgoingDiagConnections, baseQueues, NUM_OUTGOING_CONNECTIONS);
    }

private:
    ::etl::pool<IncomingDiagConnection, NUM_INCOMING_CONNECTIONS> fIncomingDiagConnectionPool;
    ManagedOutgoingDiagConnection fOutgoingDiagConnections[NUM_OUTGOING_CONNECTIONS];
    ::etl::queue<transport::TransportJob, MAX_NUM_INCOMING_MESSAGES> fSendJobQueue;
    ::etl::queue<transport::TransportJob, MAX_NUM_INCOMING_MESSAGES>
        fResponseQueues[NUM_OUTGOING_CONNECTIONS];
};

} // namespace uds
