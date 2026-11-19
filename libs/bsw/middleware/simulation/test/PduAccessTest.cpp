/*******************************************************************************
 *
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************
 */

#include "middleware/shm/Config.h"
#include "middleware/shm/PduDefinitions.h"
#include "middleware/shm/QueueDefinitions.h"
#include "middleware/sig/core/IPduTxSink.h"

#include <etl/array.h>
#include <etl/span.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <new>
#include <utility>
#include <vector>

// Exercises the code generated from model/deployment-test.yaml: PDU table, load, store, broker.
namespace
{
using namespace middleware::shm;
using middleware::sig::core::PduAccess;

struct Transmission
{
    uint32_t id;
    std::vector<uint8_t> payload;
};

class FakeBus : public middleware::sig::core::IPduTxSink
{
public:
    void submit(uint32_t const id, ::etl::span<uint8_t const> const payload) override
    {
        sent.push_back({id, std::vector<uint8_t>(payload.begin(), payload.end())});
    }

    std::vector<Transmission> sent;
};

std::vector<uint8_t> makePayload(size_t const size, uint8_t const seed)
{
    std::vector<uint8_t> payload(size);
    for (size_t i = 0U; i < size; ++i)
    {
        payload[i] = static_cast<uint8_t>(seed + i);
    }
    return payload;
}

PduAccess const* findAccess(uint32_t const id)
{
    auto const table = getPduAccessTable();
    auto const it    = std::find_if(
        table.begin(), table.end(), [id](PduAccess const& access) { return access.id == id; });
    return it == table.end() ? nullptr : &*it;
}

std::vector<uint8_t> loadPdu(PduAccess const& access)
{
    ::etl::array<uint8_t, MAX_PDU_SIZE> buffer{};
    auto const loaded = access.load(::etl::span<uint8_t>(buffer));
    return std::vector<uint8_t>(loaded.begin(), loaded.end());
}

class PduAccessTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        auto& layout = getMemoryLayout();
        layout.~MemoryLayout();
        new (&layout) MemoryLayout();
    }

    FakeBus bus;
};

TEST_F(PduAccessTest, TableIsSortedByIdAndHoldsEveryDeploymentPdu)
{
    auto const table = getPduAccessTable();

    ASSERT_EQ(table.size(), 4U);
    EXPECT_EQ(table[0].id, PDU_ID_SAMPLEPDUWITHDATATYPES);
    EXPECT_EQ(table[1].id, PDU_ID_SAMPLEPDUWITHDATATYPESFROMBUS);
    EXPECT_EQ(table[2].id, PDU_ID_SAMPLEPDUWITHCRCFROMBUS);
    EXPECT_EQ(table[3].id, PDU_ID_SAMPLEPDUWITHCRC);
}

TEST_F(PduAccessTest, EveryIdIsMappedToThePduOfThatSize)
{
    EXPECT_EQ(loadPdu(*findAccess(PDU_ID_SAMPLEPDUWITHCRC)).size(), 8U);
    EXPECT_EQ(loadPdu(*findAccess(PDU_ID_SAMPLEPDUWITHCRCFROMBUS)).size(), 8U);
    EXPECT_EQ(loadPdu(*findAccess(PDU_ID_SAMPLEPDUWITHDATATYPES)).size(), 20U);
    EXPECT_EQ(loadPdu(*findAccess(PDU_ID_SAMPLEPDUWITHDATATYPESFROMBUS)).size(), 20U);
}

TEST_F(PduAccessTest, StoreThenLoadRoundTripsForEveryPdu)
{
    for (PduAccess const& access : getPduAccessTable())
    {
        auto const size    = loadPdu(access).size();
        auto const payload = makePayload(size, static_cast<uint8_t>(access.id));

        ASSERT_TRUE(access.store(::etl::span<uint8_t const>(payload.data(), payload.size())));
        EXPECT_EQ(loadPdu(access), payload) << "id " << access.id;
    }
}

TEST_F(PduAccessTest, StoreRejectsAWrongSizeAndKeepsThePdu)
{
    for (PduAccess const& access : getPduAccessTable())
    {
        auto const size     = loadPdu(access).size();
        auto const original = makePayload(size, 0x10U);
        ASSERT_TRUE(access.store(::etl::span<uint8_t const>(original.data(), original.size())));

        auto const tooShort = makePayload(size - 1U, 0x80U);
        auto const tooLong  = makePayload(size + 1U, 0x80U);
        EXPECT_FALSE(access.store(::etl::span<uint8_t const>(tooShort.data(), tooShort.size())));
        EXPECT_FALSE(access.store(::etl::span<uint8_t const>(tooLong.data(), tooLong.size())));
        EXPECT_FALSE(access.store(::etl::span<uint8_t const>()));
        EXPECT_EQ(loadPdu(access), original) << "id " << access.id;
    }
}

TEST_F(PduAccessTest, LoadLeavesTheBufferBeyondThePduUntouched)
{
    ::etl::array<uint8_t, MAX_PDU_SIZE> buffer{};
    buffer.fill(0xAAU);

    auto const loaded = findAccess(PDU_ID_SAMPLEPDUWITHCRC)->load(::etl::span<uint8_t>(buffer));

    ASSERT_EQ(loaded.size(), 8U);
    for (size_t i = loaded.size(); i < buffer.size(); ++i)
    {
        EXPECT_EQ(buffer[i], 0xAAU) << "byte " << i;
    }
}

TEST_F(PduAccessTest, BrokerStoresReceivedPayloadInThePduOfThatId)
{
    PduBrokerConnection broker(bus, getPduAccessTable(), *getOutboundPduQueue());
    auto const crc       = makePayload(8U, 0x01U);
    auto const datatypes = makePayload(20U, 0x41U);

    EXPECT_TRUE(broker.onReceive(
        PDU_ID_SAMPLEPDUWITHCRCFROMBUS, ::etl::span<uint8_t const>(crc.data(), crc.size())));
    EXPECT_TRUE(broker.onReceive(
        PDU_ID_SAMPLEPDUWITHDATATYPESFROMBUS,
        ::etl::span<uint8_t const>(datatypes.data(), datatypes.size())));

    EXPECT_EQ(loadPdu(*findAccess(PDU_ID_SAMPLEPDUWITHCRCFROMBUS)), crc);
    EXPECT_EQ(loadPdu(*findAccess(PDU_ID_SAMPLEPDUWITHDATATYPESFROMBUS)), datatypes);
}

TEST_F(PduAccessTest, BrokerRejectsIdsThatAreNotInTheDeployment)
{
    PduBrokerConnection broker(bus, getPduAccessTable(), *getOutboundPduQueue());
    auto const payload = makePayload(8U, 0U);

    for (uint32_t const id : {0U, 9U, 11U, 25U, 41U, 0xFFFFFFFFU})
    {
        EXPECT_FALSE(
            broker.onReceive(id, ::etl::span<uint8_t const>(payload.data(), payload.size())))
            << "id " << id;
    }
}

TEST_F(PduAccessTest, BrokerTransmitsCommittedPdusInCommitOrderThroughTheQueue)
{
    PduBrokerConnection broker(bus, getPduAccessTable(), *getOutboundPduQueue());
    auto const crc       = makePayload(8U, 0x01U);
    auto const datatypes = makePayload(20U, 0x41U);
    OutboundPduQueue::Sender sender(*getOutboundPduQueue());

    ASSERT_TRUE(findAccess(PDU_ID_SAMPLEPDUWITHCRC)
                    ->store(::etl::span<uint8_t const>(crc.data(), crc.size())));
    ASSERT_TRUE(findAccess(PDU_ID_SAMPLEPDUWITHDATATYPES)
                    ->store(::etl::span<uint8_t const>(datatypes.data(), datatypes.size())));
    ASSERT_TRUE(sender.write(PDU_ID_SAMPLEPDUWITHDATATYPES));
    ASSERT_TRUE(sender.write(PDU_ID_SAMPLEPDUWITHCRC));

    broker.processTransmissions();

    ASSERT_EQ(bus.sent.size(), 2U);
    EXPECT_EQ(bus.sent[0].id, PDU_ID_SAMPLEPDUWITHDATATYPES);
    EXPECT_EQ(bus.sent[0].payload, datatypes);
    EXPECT_EQ(bus.sent[1].id, PDU_ID_SAMPLEPDUWITHCRC);
    EXPECT_EQ(bus.sent[1].payload, crc);
    EXPECT_EQ(sender.size(), 0U);
}

} // namespace
