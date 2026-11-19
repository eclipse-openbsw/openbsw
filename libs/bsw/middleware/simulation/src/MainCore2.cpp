/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/
#include <chrono>
#include <thread>

#include <etl/array.h>
#include <etl/crc8_ccitt.h>

#include "Logger.h"
#include "middleware/Cluster2Cluster.h"
#include "middleware/shm/PduDefinitions.h"
#include "middleware/shm/QueueDefinitions.h"
#include "middleware/sig/core/PduBrokerConnection.h"

namespace
{

char const* pduName(uint32_t const id)
{
    switch (id)
    {
        case middleware::shm::PDU_ID_SAMPLEPDUWITHCRC:
        {
            return "samplePduWithCrc";
        }
        case middleware::shm::PDU_ID_SAMPLEPDUWITHDATATYPES:
        {
            return "samplePduWithDatatypes";
        }
        case middleware::shm::PDU_ID_SAMPLEPDUWITHCRCFROMBUS:
        {
            return "samplePduWithCrcFromBus";
        }
        case middleware::shm::PDU_ID_SAMPLEPDUWITHDATATYPESFROMBUS:
        {
            return "samplePduWithDatatypesFromBus";
        }
        default:
        {
            return "unknown";
        }
    }
}

// Stands in for the bus driver.
class SimPduTxSink final : public middleware::sig::core::IPduTxSink
{
public:
    void submit(uint32_t const id, etl::span<uint8_t const> const payload) override
    {
        simulation::Logger::log(
            "[PduBroker] Bus Tx PDU ", id, " (", pduName(id), "), ", payload.size(), " bytes");
    }
};

template<size_t Size>
void receive(
    middleware::sig::core::PduBrokerConnectionBase& broker,
    uint32_t const id,
    etl::array<uint8_t, Size> const& payload)
{
    if (broker.onReceive(id, payload))
    {
        simulation::Logger::log("[PduBroker] Bus Rx PDU ", id, " (", pduName(id), ")");
    }
}

} // namespace

void run_main_core2()
{
    middleware::initializeCluster2ClusterConnection();

    SimPduTxSink simPduTxSink{};
    middleware::shm::PduBrokerConnection pduBrokerConnection{
        simPduTxSink,
        middleware::shm::getPduAccessTable(),
        *middleware::shm::getOutboundPduQueue()};

    size_t counter    = 0U;
    uint8_t rxCounter = 0U;
    for (;;)
    {
        if (counter > 0 && counter % 600 == 0)
        {
            break; // Run simulation for 60 seconds.
        }

        if (counter % 50 == 0)
        {
            // Simulate PDUs received from the bus every 5 seconds.
            ++rxCounter;

            // The CRC in the first byte covers the seven bytes after it.
            etl::array<uint8_t, 8U> crcPayload{
                0x00U, 0xCAU, 0xFEU, 0xBAU, 0x00U, 0x00U, 0x00U, rxCounter};
            crcPayload[0] = etl::crc8_ccitt(crcPayload.begin() + 1, crcPayload.end());
            receive(
                pduBrokerConnection, middleware::shm::PDU_ID_SAMPLEPDUWITHCRCFROMBUS, crcPayload);

            // firstSignal 0x0A, secondSignal 0x0123456789ABCDE, arraySignal {0x12, 0x34},
            // enumSignal Value1, bitsetSignal fault, locked, upperWord and highest, boolSignal
            // false.
            receive(
                pduBrokerConnection,
                middleware::shm::PDU_ID_SAMPLEPDUWITHDATATYPESFROMBUS,
                etl::array<uint8_t, 20U>{0x50U, 0x09U, 0x1AU, 0x2BU, 0x3CU, 0x4DU, 0x5EU,
                                         0x6FU, 0x09U, 0x1AU, 0x01U, 0x80U, 0x00U, 0x00U,
                                         0x01U, 0x00U, 0x00U, 0x00U, 0x0AU, 0x00U});
        }

        if (counter % 150 == 0)
        {
            auto const pduIdStats = middleware::shm::getOutboundPduQueue()->getStats();
            simulation::Logger::log(
                "[PduBroker] PDU ids [ Received:",
                pduIdStats.processedMessages,
                ", Lost: ",
                pduIdStats.lostMessages,
                ", Unknown: ",
                pduBrokerConnection.getUnknownTxIds(),
                " ]");
        }

        counter++;

        middleware::processCluster2Cluster(10);
        pduBrokerConnection.processTransmissions();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
