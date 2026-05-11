/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "routing/Router.h"

#include "routing/definition.h"

#include "routing/PduRoutingTable.h"

#include <blob/Blob.h>
#include <etl/array.h>
#include <etl/span.h>
#include <etl/vector.h>
#include <io/MemoryQueue.h>

#include <gmock/gmock.h>

namespace
{
using namespace ::testing;

template<size_t MAX_NUM_CHANNELS>
struct RouterWithDefinitions
{
    static constexpr size_t CAPACITY         = 1024;
    static constexpr size_t MAX_ELEMENT_SIZE = 128;

    using Router = ::routing::Router<MAX_NUM_CHANNELS>;

    using Queue  = ::io::MemoryQueue<CAPACITY, MAX_ELEMENT_SIZE>;
    using Reader = ::io::MemoryQueueReader<Queue>;
    using Writer = ::io::MemoryQueueWriter<Queue>;

    RouterWithDefinitions()
    {
        for (size_t i = 0U; i < MAX_NUM_CHANNELS; ++i)
        {
            auto& rxQueue = rxQueues.emplace_back();
            auto& txQueue = txQueues.emplace_back();
            (void)rxReaders.emplace_back(rxQueue);
            (void)txReaders.emplace_back(txQueue);
            (void)rxWriters.emplace_back(rxQueue);
            (void)txWriters.emplace_back(txQueue);

            auto& reader = rxReaders[i];
            readers[i]   = &reader;
            auto& writer = txWriters[i];
            writers[i]   = &writer;
        }
    }

    void init(
        ::etl::span<::routing::Definition> defs,
        ::etl::span<::io::IReader*> readers,
        ::etl::span<::io::IWriter*> writers)
    {
        ::routing::load(pduRoutingTable, defs, routingMem);
        router.init(pduRoutingTable, readers, writers);
    }

    void init(::etl::span<::routing::Definition> defs) { init(defs, readers, writers); }

    void init(::etl::span<uint8_t const> const config) { router.init(config, readers, writers); }

    bool run() { return router.run(); }

    Router router;

    ::etl::vector<Queue, MAX_NUM_CHANNELS> rxQueues;
    ::etl::vector<Queue, MAX_NUM_CHANNELS> txQueues;
    ::etl::vector<Reader, MAX_NUM_CHANNELS> rxReaders;
    ::etl::vector<Reader, MAX_NUM_CHANNELS> txReaders;
    ::etl::vector<Writer, MAX_NUM_CHANNELS> rxWriters;
    ::etl::vector<Writer, MAX_NUM_CHANNELS> txWriters;
    ::etl::array<::io::IReader*, MAX_NUM_CHANNELS> readers;
    ::etl::array<::io::IWriter*, MAX_NUM_CHANNELS> writers;

    ::routing::RoutingTableMem routingMem;
    ::routing::PduRoutingTable pduRoutingTable;
};

void writePdu(uint32_t const id, size_t const size, ::io::IWriter& writer)
{
    auto pdu = writer.allocate(size);
    if (!pdu.empty() && pdu.size() >= 2 * sizeof(::etl::be_uint32_t))
    {
        pdu.take<::etl::be_uint32_t>() = id;
        auto& length                   = pdu.take<::etl::be_uint32_t>();
        length                         = pdu.size();
        writer.commit();
    }
}

/**
 * \desc
 * Routing a single PDU from a source channel to an output channel works
 */
TEST(RouterTest, route_single_pdu)
{
    constexpr size_t MAX_NUM_CHANNELS = 2;
    ::routing::Definition defs[] = {::routing::Definition().in(0, 0x34, 0, 4).out(1, 0x43, 0, 4)};
    RouterWithDefinitions<MAX_NUM_CHANNELS> router;
    router.init(defs);

    // We need to use internalPduId, so 0
    writePdu(0U, 12, router.rxWriters[0]);

    ASSERT_TRUE(router.run());
    ASSERT_EQ(0, router.txReaders[0].peek().size());
    ASSERT_EQ(12, router.txReaders[1].peek().size()); // id + length + payload size
    EXPECT_EQ(0x43U, router.txReaders[1].peek().take<::etl::be_uint32_t>());
    router.txReaders[1].release();
}

/**
 * \desc
 * PDU is discarded and not routed to any channel if its size is invalid
 */
TEST(RouterTest, discard_pdu_invalid_size)
{
    constexpr size_t MAX_NUM_CHANNELS = 2;
    ::routing::Definition defs[]      = {::routing::Definition().in(0, 0, 0, 4).out(1, 1, 0, 4)};
    RouterWithDefinitions<MAX_NUM_CHANNELS> router;
    router.init(defs);

    writePdu(1, sizeof(uint32_t) - 1, router.rxWriters[0]);
    ASSERT_FALSE(router.run());
    ASSERT_EQ(0, router.rxReaders[0].peek().size());
    ASSERT_EQ(0, router.txReaders[1].peek().size());
}

/**
 * \desc
 * Unknown PDUs are discarded and not routed to any channel
 */
TEST(RouterTest, unknown_pdus_are_not_routed)
{
    constexpr size_t MAX_NUM_CHANNELS = 4;
    ::routing::Definition defs[]
        = {::routing::Definition().in(0, 0x34, 0, 4).out(1, 0x01, 0, 4),
           ::routing::Definition().in(1, 0x56, 0, 8).out(2, 0x02, 0, 8),
           ::routing::Definition().in(2, 0x78, 0, 8).out(3, 0x03, 0, 8)};
    RouterWithDefinitions<MAX_NUM_CHANNELS> router;
    router.init(defs);

    // write unknown PDU to each channel
    for (size_t i = 0; i < MAX_NUM_CHANNELS; i++)
    {
        writePdu(0x7U + i, 16, router.rxWriters[i]);
    }

    ASSERT_FALSE(router.run());

    for (size_t i = 0; i < MAX_NUM_CHANNELS; i++)
    {
        ASSERT_EQ(0, router.rxReaders[i].peek().size());
        ASSERT_EQ(0, router.txReaders[i].peek().size());
    }
}

/**
 * \desc
 * Routing PDUs to multiple channels works
 */
TEST(RouterTest, route_to_multiple_channels)
{
    constexpr size_t MAX_NUM_CHANNELS = 5;
    ::routing::Definition defs[]      = {::routing::Definition()
                                             .in(0, 0x34, 0, 4)
                                             .out(1, 0x01, 0, 4)
                                             .out(2, 0x02, 0, 4)
                                             .out(3, 0x03, 0, 4)
                                             .out(4, 0x04, 0, 4)};
    RouterWithDefinitions<MAX_NUM_CHANNELS> router;
    router.init(defs);

    writePdu(0, 12, router.rxWriters[0]);
    ASSERT_TRUE(router.run());
    ASSERT_EQ(0, router.rxReaders[0].peek().size());
    for (size_t i = 1; i < MAX_NUM_CHANNELS; i++)
    {
        ASSERT_EQ(12, router.txReaders[i].peek().size());
    }
}

/**
 * \desc
 * PDUs are routed in a round robin fashion.
 */
TEST(RouterTest, route_pdus_round_robin)
{
    constexpr size_t MAX_NUM_CHANNELS = 3;
    ::routing::Definition defs[]
        = {::routing::Definition().in(0, 0, 0, 4).out(1, 10, 0, 4),
           ::routing::Definition().in(1, 1, 0, 4).out(2, 11, 0, 4),
           ::routing::Definition().in(2, 2, 0, 4).out(0, 12, 0, 4)};
    RouterWithDefinitions<MAX_NUM_CHANNELS> router;
    router.init(defs);

    writePdu(0, 12, router.rxWriters[0]);
    writePdu(0, 12, router.rxWriters[0]);
    writePdu(1, 12, router.rxWriters[1]);
    writePdu(1, 12, router.rxWriters[1]);
    writePdu(2, 12, router.rxWriters[2]);
    writePdu(2, 12, router.rxWriters[2]);

    ASSERT_TRUE(router.run());
    ASSERT_EQ(12, router.txReaders[1].peek().size());

    ASSERT_TRUE(router.run());
    ASSERT_EQ(12, router.txReaders[2].peek().size());

    ASSERT_TRUE(router.run());
    ASSERT_EQ(12, router.txReaders[0].peek().size());

    router.txReaders[0].release();
    router.txReaders[1].release();
    router.txReaders[2].release();

    ASSERT_EQ(0, router.txReaders[0].peek().size());
    ASSERT_EQ(0, router.txReaders[1].peek().size());
    ASSERT_EQ(0, router.txReaders[2].peek().size());

    ASSERT_TRUE(router.run());
    ASSERT_EQ(12, router.txReaders[1].peek().size());

    ASSERT_TRUE(router.run());
    ASSERT_EQ(12, router.txReaders[2].peek().size());

    ASSERT_TRUE(router.run());
    ASSERT_EQ(12, router.txReaders[0].peek().size());
}

/**
 * \desc
 * Routing all PDUs from defintion works
 */
TEST(RouterTest, all_pdus)
{
    constexpr size_t HEADER_SIZE      = 8;
    constexpr size_t MAX_NUM_CHANNELS = 5;
    ::routing::Definition defs[]
        = {::routing::Definition().in(0, 0x34, 3, 2).out(1, 0x44, 0, 2),
           ::routing::Definition().in(0, 0x34, 0, 7).out(1, 0x45, 0, 7),
           ::routing::Definition().in(0, 0x34, 0, 3).out(1, 0x43, 0, 3),
           ::routing::Definition().in(0, 0x35, 0, 8).out(1, 0x46, 0, 8),
           ::routing::Definition().in(0, 0x36, 0, 9).out(1, 0x47, 0, 9),
           ::routing::Definition().in(1, 0x3, 8, 16).out(2, 0x5, 0, 16),
           ::routing::Definition().in(1, 0x3, 0, 8).out(2, 0x4, 0, 8),
           ::routing::Definition().in(2, 0x4, 0, 3).out(1, 0x34, 0, 3).out(4, 0x12, 0, 3),
           ::routing::Definition().in(2, 0x5, 0, 1).out(0, 0x4, 0, 1),
           ::routing::Definition().in(3, 0x33, 0, 3).out(2, 0x22, 0, 3),
           ::routing::Definition().in(3, 0x34, 0, 3).out(4, 0x45, 0, 3),
           ::routing::Definition().in(4, 0x44, 0, 4).out(3, 0x33, 0, 4)};

    RouterWithDefinitions<MAX_NUM_CHANNELS> router;
    router.init(defs);

    uint32_t internalPduId = 0;
    ::etl::vector<::routing::DefinitionRef, 1024> definitions;
    for (auto const& i : defs)
    {
        definitions.push_back({&i});
    }
    ::routing::sort(definitions);
    for (auto const definition : definitions)
    {
        auto def = *definition.d;

        // converting from messageId to internalPduId is the job of the RxAdapter
        writePdu(
            internalPduId, def.input.length + HEADER_SIZE, router.rxWriters[def.input.channelId]);
        ASSERT_TRUE(router.run());

        for (auto const& output : def.outputs)
        {
            // the length isn't modified by the routing core, this is the job of the txAdapter
            auto& reader = router.txReaders[output.channelId];
            ASSERT_EQ(def.input.length + HEADER_SIZE, reader.peek().size())
                << internalPduId << " : " << (uint32_t)def.input.channelId << " : "
                << (uint32_t)output.channelId;
            EXPECT_EQ(output.messageId, reader.peek().take<::etl::be_uint32_t>());
            reader.release();
        }

        internalPduId++;
    }
}

/**
 * \desc
 * Routing PDUs from defintion works with a smaller number of channels
 */
TEST(RouterTest, all_pdus_smaller_number_of_channels)
{
    constexpr size_t HEADER_SIZE       = 8;
    constexpr uint8_t MAX_NUM_CHANNELS = 5;
    constexpr uint8_t NUM_CHANNELS     = MAX_NUM_CHANNELS - 1;
    ::routing::Definition defs[]
        = {::routing::Definition().in(0, 0x34, 3, 2).out(1, 0x44, 0, 2),
           ::routing::Definition().in(0, 0x34, 0, 7).out(1, 0x45, 0, 7),
           ::routing::Definition().in(0, 0x34, 0, 3).out(1, 0x43, 0, 3),
           ::routing::Definition().in(0, 0x35, 0, 8).out(1, 0x46, 0, 8),
           ::routing::Definition().in(0, 0x36, 0, 9).out(1, 0x47, 0, 9),
           ::routing::Definition().in(1, 0x3, 8, 16).out(2, 0x5, 0, 16),
           ::routing::Definition().in(1, 0x3, 0, 8).out(2, 0x4, 0, 8),
           ::routing::Definition().in(2, 0x4, 0, 3).out(1, 0x34, 0, 3).out(4, 0x12, 0, 3),
           ::routing::Definition().in(2, 0x5, 0, 1).out(0, 0x4, 0, 1),
           ::routing::Definition().in(3, 0x33, 0, 3).out(2, 0x22, 0, 3),
           ::routing::Definition().in(3, 0x34, 0, 3).out(4, 0x45, 0, 3),
           ::routing::Definition().in(4, 0x44, 0, 4).out(3, 0x33, 0, 4)};

    RouterWithDefinitions<MAX_NUM_CHANNELS> router;

    auto readers = ::etl::make_span(router.readers).first(NUM_CHANNELS);
    auto writers = ::etl::make_span(router.writers).first(NUM_CHANNELS);
    router.init(defs, readers, writers);

    uint32_t internalPduId = 0;
    ::etl::vector<::routing::DefinitionRef, 1024> definitions;
    for (auto const& i : defs)
    {
        definitions.push_back({&i});
    }
    ::routing::sort(definitions);
    for (auto const definition : definitions)
    {
        auto def = *definition.d;

        // converting from messageId to internalPduId is the job of the RxAdapter
        writePdu(
            internalPduId, def.input.length + HEADER_SIZE, router.rxWriters[def.input.channelId]);

        if (def.input.channelId >= NUM_CHANNELS)
        {
            ASSERT_FALSE(router.run());
            ASSERT_EQ(
                def.input.length + HEADER_SIZE,
                router.rxReaders[def.input.channelId].peek().size());
            router.rxReaders[def.input.channelId].release();
            continue;
        }

        ASSERT_TRUE(router.run());
        for (auto const& output : def.outputs)
        {
            // the length isn't modified by the routing core, this is the job of the txAdapter
            auto& reader = router.txReaders[output.channelId];
            if (output.channelId < NUM_CHANNELS)
            {
                ASSERT_EQ(def.input.length + HEADER_SIZE, reader.peek().size())
                    << internalPduId << " : " << (uint32_t)def.input.channelId << " : "
                    << (uint32_t)output.channelId;
                EXPECT_EQ(output.messageId, reader.peek().take<::etl::be_uint32_t>());
                reader.release();
            }
            else
            {
                ASSERT_EQ(0U, reader.peek().size());
            }
        }

        internalPduId++;
    }
}

/**
 * \desc
 * Routing PDUs from defintion works with a bigger number of channels
 */
TEST(RouterTest, all_pdus_bigger_number_of_channels)
{
    constexpr size_t HEADER_SIZE       = 8;
    constexpr uint8_t MAX_NUM_CHANNELS = 5;
    constexpr uint8_t NUM_CHANNELS     = MAX_NUM_CHANNELS + 1;
    ::routing::Definition defs[]
        = {::routing::Definition().in(0, 0x34, 3, 2).out(1, 0x44, 0, 2),
           ::routing::Definition().in(0, 0x34, 0, 7).out(1, 0x45, 0, 7),
           ::routing::Definition().in(0, 0x34, 0, 3).out(1, 0x43, 0, 3),
           ::routing::Definition().in(0, 0x35, 0, 8).out(1, 0x46, 0, 8),
           ::routing::Definition().in(0, 0x36, 0, 9).out(1, 0x47, 0, 9),
           ::routing::Definition().in(1, 0x3, 8, 16).out(2, 0x5, 0, 16),
           ::routing::Definition().in(1, 0x3, 0, 8).out(2, 0x4, 0, 8),
           ::routing::Definition().in(2, 0x4, 0, 3).out(1, 0x34, 0, 3).out(4, 0x12, 0, 3),
           ::routing::Definition().in(2, 0x5, 0, 1).out(0, 0x4, 0, 1),
           ::routing::Definition().in(3, 0x33, 0, 3).out(2, 0x22, 0, 3),
           ::routing::Definition().in(3, 0x34, 0, 3).out(4, 0x45, 0, 3),
           ::routing::Definition().in(4, 0x44, 0, 4).out(3, 0x33, 0, 4).out(5, 0x55, 0, 4),
           ::routing::Definition().in(4, 0x45, 0, 4).out(5, 0x56, 0, 4),
           ::routing::Definition().in(5, 0x55, 0, 5).out(4, 0x44, 0, 5)};

    RouterWithDefinitions<MAX_NUM_CHANNELS> router;

    decltype(router)::Queue rxQueue;
    decltype(router)::Queue txQueue;
    decltype(router)::Reader rxReader(rxQueue);
    decltype(router)::Reader txReader(txQueue);
    decltype(router)::Writer rxWriter(rxQueue);
    decltype(router)::Writer txWriter(txQueue);
    ::etl::array<::io::IReader*, NUM_CHANNELS> readers;
    ::etl::array<::io::IWriter*, NUM_CHANNELS> writers;

    for (size_t i = 0; i < MAX_NUM_CHANNELS; ++i)
    {
        readers[i] = router.readers[i];
        writers[i] = router.writers[i];
    }
    readers.back() = &rxReader;
    writers.back() = &txWriter;

    router.init(defs, readers, writers);

    uint32_t internalPduId = 0;
    ::etl::vector<::routing::DefinitionRef, 1024> definitions;
    for (auto const& i : defs)
    {
        definitions.push_back({&i});
    }
    ::routing::sort(definitions);
    for (auto const definition : definitions)
    {
        auto def = *definition.d;

        auto& writer = def.input.channelId < MAX_NUM_CHANNELS
                           ? router.rxWriters[def.input.channelId]
                           : rxWriter;
        // converting from messageId to internalPduId is the job of the RxAdapter
        writePdu(internalPduId, def.input.length + HEADER_SIZE, writer);

        if (def.input.channelId >= MAX_NUM_CHANNELS)
        {
            ASSERT_FALSE(router.run());
            ASSERT_EQ(def.input.length + HEADER_SIZE, rxReader.peek().size());
            rxReader.release();
            continue;
        }

        ASSERT_TRUE(router.run());
        for (auto const& output : def.outputs)
        {
            // the length isn't modified by the routing core, this is the job of the txAdapter
            if (output.channelId < MAX_NUM_CHANNELS)
            {
                auto& reader = router.txReaders[output.channelId];
                ASSERT_EQ(def.input.length + HEADER_SIZE, reader.peek().size())
                    << internalPduId << " : " << (uint32_t)def.input.channelId << " : "
                    << (uint32_t)output.channelId;
                EXPECT_EQ(output.messageId, reader.peek().take<::etl::be_uint32_t>());
                reader.release();
            }
            else
            {
                ASSERT_EQ(0U, txReader.peek().size());
            }
        }

        internalPduId++;
    }
}

/**
 * \desc
 * Initializing router with empty blob doesn't crash.
 */
TEST(RouterTest, empty_blob)
{
    constexpr size_t MAX_NUM_CHANNELS = 2;
    ::etl::span<uint8_t const> emptyBlob;
    RouterWithDefinitions<MAX_NUM_CHANNELS> router;

    router.init(emptyBlob);
    router.run();
}

/**
 * \desc
 * Reinitializing router does nothing.
 */
TEST(RouterTest, init_twice)
{
    constexpr size_t MAX_NUM_CHANNELS = 2;
    ::routing::Definition defs[] = {::routing::Definition().in(0, 0x34, 0, 4).out(1, 0x43, 0, 4)};
    ::etl::span<uint8_t const> emptyBlob;

    RouterWithDefinitions<MAX_NUM_CHANNELS> router;
    router.init(defs);
    router.init(defs);
    router.init(emptyBlob);

    writePdu(0U, 12, router.rxWriters[0]);

    ASSERT_TRUE(router.run());
}

/**
 * \desc
 * Initialization fails if any reader or writer is invalid.
 */
TEST(RouterTest, invalid_reader_or_writer)
{
    constexpr size_t MAX_NUM_CHANNELS = 2;
    ::routing::Definition defs[] = {::routing::Definition().in(0, 0x34, 0, 4).out(1, 0x43, 0, 4)};
    RouterWithDefinitions<MAX_NUM_CHANNELS> router;

    auto readers    = router.readers;
    readers.front() = nullptr;
    router.init(defs, readers, router.writers);

    auto writers    = router.writers;
    writers.front() = nullptr;
    router.init(defs, router.readers, writers);

    writePdu(0U, 12, router.rxWriters[0]);

    ASSERT_FALSE(router.run());
}

/**
 * \desc
 * Initialization fails if there are no destination offsets.
 */
TEST(RouterTest, empty_destination_offsets)
{
    constexpr size_t MAX_NUM_CHANNELS = 2;

    ::routing::Definition defs[] = {::routing::Definition().in(0, 0x34, 0, 4).out(1, 0x43, 0, 4)};
    RouterWithDefinitions<MAX_NUM_CHANNELS> routerWithDefs;
    auto& table  = routerWithDefs.pduRoutingTable;
    auto& router = routerWithDefs.router;

    ::routing::load(table, defs, routerWithDefs.routingMem);
    table.destinationOffsets = {};
    router.init(table, routerWithDefs.readers, routerWithDefs.writers);

    writePdu(0U, 12, routerWithDefs.rxWriters[0]);

    ASSERT_FALSE(router.run());
}

/**
 * \desc
 * Initialization fails if there are no output message IDs.
 */
TEST(RouterTest, empty_output_message_ids)
{
    constexpr size_t MAX_NUM_CHANNELS = 2;

    ::routing::Definition defs[] = {::routing::Definition().in(0, 0x34, 0, 4).out(1, 0x43, 0, 4)};
    RouterWithDefinitions<MAX_NUM_CHANNELS> routerWithDefs;
    auto& table      = routerWithDefs.pduRoutingTable;
    auto& routingMem = routerWithDefs.routingMem;
    auto& readers    = routerWithDefs.readers;
    auto& writers    = routerWithDefs.writers;
    auto& router     = routerWithDefs.router;
    auto& rxWriters  = routerWithDefs.rxWriters;

    ::routing::load(table, defs, routingMem);
    table.destinations = {};
    router.init(table, readers, writers);

    ::routing::load(table, defs, routingMem);
    table.destinationOffsets = {};
    router.init(table, readers, writers);

    ::routing::load(table, defs, routingMem);
    table.outputMessageIds = {};
    router.init(table, readers, writers);

    writePdu(0U, 12, rxWriters[0]);

    ASSERT_FALSE(router.run());
}

/**
 * \desc
 * Initialization fails if the number of destinations and message IDs do not match.
 */
TEST(RouterTest, different_number_of_destinations_and_message_ids)
{
    constexpr size_t MAX_NUM_CHANNELS = 3;

    ::routing::Definition defs[]
        = {::routing::Definition().in(0, 0, 0, 4).out(1, 10, 0, 4),
           ::routing::Definition().in(1, 1, 0, 4).out(2, 11, 0, 4),
           ::routing::Definition().in(2, 2, 0, 4).out(0, 12, 0, 4)};
    RouterWithDefinitions<MAX_NUM_CHANNELS> routerWithDefs;
    auto& table      = routerWithDefs.pduRoutingTable;
    auto& routingMem = routerWithDefs.routingMem;
    auto& readers    = routerWithDefs.readers;
    auto& writers    = routerWithDefs.writers;
    auto& router     = routerWithDefs.router;
    auto& rxWriters  = routerWithDefs.rxWriters;

    ::routing::load(table, defs, routingMem);
    table.destinations = table.destinations.first(table.destinations.size() - 1);
    router.init(table, readers, writers);

    ::routing::load(table, defs, routingMem);
    table.outputMessageIds = table.outputMessageIds.first(table.outputMessageIds.size() - 1);
    router.init(table, readers, writers);

    writePdu(0, 12, rxWriters[0]);
    writePdu(1, 12, rxWriters[1]);
    writePdu(2, 12, rxWriters[2]);

    ASSERT_FALSE(router.run());
}

} // namespace
