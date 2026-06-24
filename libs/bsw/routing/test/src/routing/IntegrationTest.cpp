/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "routing/Integration.h"

#include "blob/configuration.h"
#include "routing/ErrorHandler.h"
#include "routing/constants.h"

#include <etl/array.h>
#include <etl/vector.h>
#include <io/MemoryQueue.h>

#include <gmock/gmock.h>

using namespace ::testing;

namespace
{
constexpr uint8_t NUM_PDU_TRANSPORT_CHANNELS = ::routing::NUM_PDU_TRANSPORT_CHANNELS;
constexpr uint8_t NUM_CAN_CHANNELS           = ::routing::NUM_CAN_CHANNELS;
constexpr uint8_t NUM_FLEXRAY_CHANNELS       = ::routing::NUM_FLEXRAY_CHANNELS;

template<size_t SIZE, typename ChannelAdapter>
::etl::array<uint8_t, SIZE> channelIds(::etl::span<ChannelAdapter const> const adapters)
{
    ::etl::array<uint8_t, SIZE> ids;
    for (size_t i = 0; i < SIZE; ++i)
    {
        ids[i] = adapters[i].channelId;
    }
    return ids;
}

class IntegrationTest : public ::testing::Test
{
public:
    static constexpr size_t MAX_ELEMENT_SIZE = 72U;
    static constexpr size_t CAPACITY         = 10 * MAX_ELEMENT_SIZE;

    using Queue  = ::io::MemoryQueue<CAPACITY, MAX_ELEMENT_SIZE>;
    using Reader = ::io::MemoryQueueReader<Queue>;
    using Writer = ::io::MemoryQueueWriter<Queue>;
    template<
        uint8_t PDU_TRANSPORT    = NUM_PDU_TRANSPORT_CHANNELS,
        uint8_t CAN              = NUM_CAN_CHANNELS,
        uint8_t FR               = NUM_FLEXRAY_CHANNELS,
        size_t MAX_PDU_TRANSPORT = MAX_ELEMENT_SIZE,
        size_t MAX_CAN           = MAX_ELEMENT_SIZE,
        size_t MAX_FR            = MAX_ELEMENT_SIZE>
    using Integration
        = ::routing::Integration<PDU_TRANSPORT, CAN, FR, MAX_PDU_TRANSPORT, MAX_CAN, MAX_FR>;

    IntegrationTest()
    {
        for (size_t i = 0; i < NUM_PDU_TRANSPORT_CHANNELS; ++i)
        {
            auto& inputQueue = _pduTransportInputQueues.emplace_back();
            auto& reader     = _pduTransportInputReaders.emplace_back(inputQueue);
            (void)_pduTransportInputWriters.emplace_back(inputQueue);
            _pduTransportReaders[i] = &reader;

            auto& outputQueue = _pduTransportOutputQueues.emplace_back();
            (void)_pduTransportOutputReaders.emplace_back(outputQueue);
            auto& writer            = _pduTransportOutputWriters.emplace_back(outputQueue);
            _pduTransportWriters[i] = &writer;

            _pduTransportChannelIds[i] = NUM_CAN_CHANNELS + NUM_FLEXRAY_CHANNELS + i;
        }

        for (size_t i = 0; i < NUM_CAN_CHANNELS; ++i)
        {
            auto& inputQueue = _canInputQueues.emplace_back();
            auto& reader     = _canInputReaders.emplace_back(inputQueue);
            (void)_canInputWriters.emplace_back(inputQueue);
            _canReaders[i] = &reader;

            auto& outputQueue = _canOutputQueues.emplace_back();
            (void)_canOutputReaders.emplace_back(outputQueue);
            auto& writer   = _canOutputWriters.emplace_back(outputQueue);
            _canWriters[i] = &writer;
        }

        for (size_t i = 0; i < NUM_FLEXRAY_CHANNELS; ++i)
        {
            auto& inputQueue = _frInputQueues.emplace_back();
            auto& reader     = _frInputReaders.emplace_back(inputQueue);
            (void)_frInputWriters.emplace_back(inputQueue);
            _frReaders[i] = &reader;

            auto& outputQueue = _frOutputQueues.emplace_back();
            (void)_frOutputReaders.emplace_back(outputQueue);
            auto& writer  = _frOutputWriters.emplace_back(outputQueue);
            _frWriters[i] = &writer;
        }
    }

protected:
    ::etl::vector<Queue, NUM_PDU_TRANSPORT_CHANNELS> _pduTransportInputQueues;
    ::etl::vector<Queue, NUM_PDU_TRANSPORT_CHANNELS> _pduTransportOutputQueues;

    ::etl::vector<Queue, NUM_CAN_CHANNELS> _canInputQueues;
    ::etl::vector<Queue, NUM_CAN_CHANNELS> _canOutputQueues;

    ::etl::vector<Queue, NUM_FLEXRAY_CHANNELS> _frInputQueues;
    ::etl::vector<Queue, NUM_FLEXRAY_CHANNELS> _frOutputQueues;

    ::etl::vector<Reader, NUM_PDU_TRANSPORT_CHANNELS> _pduTransportInputReaders;
    ::etl::vector<Writer, NUM_PDU_TRANSPORT_CHANNELS> _pduTransportInputWriters;
    ::etl::vector<Reader, NUM_PDU_TRANSPORT_CHANNELS> _pduTransportOutputReaders;
    ::etl::vector<Writer, NUM_PDU_TRANSPORT_CHANNELS> _pduTransportOutputWriters;

    ::etl::vector<Reader, NUM_CAN_CHANNELS> _canInputReaders;
    ::etl::vector<Writer, NUM_CAN_CHANNELS> _canInputWriters;
    ::etl::vector<Reader, NUM_CAN_CHANNELS> _canOutputReaders;
    ::etl::vector<Writer, NUM_CAN_CHANNELS> _canOutputWriters;

    ::etl::vector<Reader, NUM_FLEXRAY_CHANNELS> _frInputReaders;
    ::etl::vector<Writer, NUM_FLEXRAY_CHANNELS> _frInputWriters;
    ::etl::vector<Reader, NUM_FLEXRAY_CHANNELS> _frOutputReaders;
    ::etl::vector<Writer, NUM_FLEXRAY_CHANNELS> _frOutputWriters;

    ::etl::array<::io::IReader*, NUM_PDU_TRANSPORT_CHANNELS> _pduTransportReaders;
    ::etl::array<::io::IWriter*, NUM_PDU_TRANSPORT_CHANNELS> _pduTransportWriters;

    ::etl::array<::io::IReader*, NUM_CAN_CHANNELS> _canReaders;
    ::etl::array<::io::IWriter*, NUM_CAN_CHANNELS> _canWriters;

    ::etl::array<::io::IReader*, NUM_FLEXRAY_CHANNELS> _frReaders;
    ::etl::array<::io::IWriter*, NUM_FLEXRAY_CHANNELS> _frWriters;

    ::etl::array<uint8_t, NUM_PDU_TRANSPORT_CHANNELS> _pduTransportChannelIds;
};

void writePdu(uint32_t const id, size_t const size, ::io::IWriter& writer)
{
    auto pdu = writer.allocate(size);
    if (!pdu.empty())
    {
        pdu.take<::etl::be_uint32_t>() = id;
        auto& length                   = pdu.take<::etl::be_uint32_t>();
        length                         = pdu.size();
        writer.commit();
    }
}

/**
 * \desc: Initializing integration creates the correct number of adapters.
 */
TEST_F(IntegrationTest, number_of_adapters)
{
    Integration<> integration;

    EXPECT_EQ(0, integration.pduTransportChannelAdapters().size());
    EXPECT_EQ(0, integration.canChannelAdapters().size());
    EXPECT_EQ(0, integration.flexrayChannelAdapters().size());

    integration.init(
        ::blob::CONFIGURATION_BLOB,
        ::etl::make_span(_pduTransportReaders),
        ::etl::make_span(_pduTransportWriters),
        ::routing::canChannelIds,
        _canReaders,
        _canWriters,
        ::routing::frChannelIds,
        _frReaders,
        _frWriters,
        {});

    EXPECT_EQ(NUM_PDU_TRANSPORT_CHANNELS, integration.pduTransportChannelAdapters().size());
    EXPECT_EQ(NUM_CAN_CHANNELS, integration.canChannelAdapters().size());
    EXPECT_EQ(NUM_FLEXRAY_CHANNELS, integration.flexrayChannelAdapters().size());

    EXPECT_THAT(
        channelIds<NUM_CAN_CHANNELS>(integration.canChannelAdapters()),
        ElementsAreArray(::routing::canChannelIds));
    EXPECT_THAT(
        channelIds<NUM_FLEXRAY_CHANNELS>(integration.flexrayChannelAdapters()),
        ElementsAreArray(::routing::frChannelIds));
    EXPECT_THAT(
        channelIds<NUM_PDU_TRANSPORT_CHANNELS>(integration.pduTransportChannelAdapters()),
        ElementsAreArray(_pduTransportChannelIds));
}

/**
 * \desc: Initializing integration with empty blob doesn't crash.
 */
TEST_F(IntegrationTest, empty_blob)
{
    Integration<> integration;
    ::etl::span<uint8_t const> emptyBlob;

    integration.init(
        emptyBlob,
        ::etl::make_span(_pduTransportReaders),
        ::etl::make_span(_pduTransportWriters),
        ::routing::canChannelIds,
        _canReaders,
        _canWriters,
        ::routing::frChannelIds,
        _frReaders,
        _frWriters,
        {});

    EXPECT_EQ(0, integration.pduTransportChannelAdapters().size());
    EXPECT_EQ(0, integration.canChannelAdapters().size());
    EXPECT_EQ(0, integration.flexrayChannelAdapters().size());

    integration.route();
}

/**
 * \desc: Initializing integration with a smaller number of PDU transport channels doesn't crash.
 */
TEST_F(IntegrationTest, init_smaller_number_of_pdu_transport_channels)
{
    constexpr auto PDU_TRANSPORT_CHANNELS = NUM_PDU_TRANSPORT_CHANNELS - 1;

    Integration<> integration;

    integration.init(
        ::blob::CONFIGURATION_BLOB,
        ::etl::make_span(_pduTransportReaders).first(PDU_TRANSPORT_CHANNELS),
        ::etl::make_span(_pduTransportWriters).first(PDU_TRANSPORT_CHANNELS),
        ::routing::canChannelIds,
        _canReaders,
        _canWriters,
        ::routing::frChannelIds,
        _frReaders,
        _frWriters,
        {});

    EXPECT_EQ(PDU_TRANSPORT_CHANNELS, integration.pduTransportChannelAdapters().size());
    EXPECT_EQ(NUM_CAN_CHANNELS, integration.canChannelAdapters().size());
    EXPECT_EQ(NUM_FLEXRAY_CHANNELS, integration.flexrayChannelAdapters().size());

    EXPECT_THAT(
        channelIds<NUM_CAN_CHANNELS>(integration.canChannelAdapters()),
        ElementsAreArray(::routing::canChannelIds));
    EXPECT_THAT(
        channelIds<NUM_FLEXRAY_CHANNELS>(integration.flexrayChannelAdapters()),
        ElementsAreArray(::routing::frChannelIds));
    EXPECT_THAT(
        channelIds<PDU_TRANSPORT_CHANNELS>(integration.pduTransportChannelAdapters()),
        ElementsAreArray(::etl::make_span(_pduTransportChannelIds).first(PDU_TRANSPORT_CHANNELS)));

    integration.route();
}

/**
 * \desc: Initializing integration with a bigger number of PDU transport channels doesn't crash.
 */
TEST_F(IntegrationTest, init_bigger_number_of_pdu_transport_channels)
{
    constexpr auto PDU_TRANSPORT_CHANNELS = NUM_PDU_TRANSPORT_CHANNELS + 1;

    ::etl::array<io::IReader*, PDU_TRANSPORT_CHANNELS> pduTransportReaders;
    ::etl::array<io::IWriter*, PDU_TRANSPORT_CHANNELS> pduTransportWriters;

    for (size_t i = 0; i < NUM_PDU_TRANSPORT_CHANNELS; ++i)
    {
        pduTransportReaders[i] = _pduTransportReaders[i];
        pduTransportWriters[i] = _pduTransportWriters[i];
    }
    pduTransportReaders.back() = nullptr;
    pduTransportWriters.back() = nullptr;

    Integration<> integration;

    integration.init(
        ::blob::CONFIGURATION_BLOB,
        ::etl::make_span(pduTransportReaders),
        ::etl::make_span(pduTransportWriters),
        ::routing::canChannelIds,
        _canReaders,
        _canWriters,
        ::routing::frChannelIds,
        _frReaders,
        _frWriters,
        {});

    EXPECT_EQ(NUM_PDU_TRANSPORT_CHANNELS, integration.pduTransportChannelAdapters().size());
    EXPECT_EQ(NUM_CAN_CHANNELS, integration.canChannelAdapters().size());
    EXPECT_EQ(NUM_FLEXRAY_CHANNELS, integration.flexrayChannelAdapters().size());

    EXPECT_THAT(
        channelIds<NUM_CAN_CHANNELS>(integration.canChannelAdapters()),
        ElementsAreArray(::routing::canChannelIds));
    EXPECT_THAT(
        channelIds<NUM_FLEXRAY_CHANNELS>(integration.flexrayChannelAdapters()),
        ElementsAreArray(::routing::frChannelIds));
    EXPECT_THAT(
        channelIds<NUM_PDU_TRANSPORT_CHANNELS>(integration.pduTransportChannelAdapters()),
        ElementsAreArray(_pduTransportChannelIds));

    integration.route();
}

/**
 * \desc: Initializing integration without PDU transport channels doesn't crash.
 */
TEST_F(IntegrationTest, no_pdu_transport_channels)
{
    constexpr size_t PDU_TRANSPORT_CHANNELS = 0;
    constexpr auto CAN_CHANNELS             = NUM_CAN_CHANNELS;
    constexpr auto FR_CHANNELS              = NUM_FLEXRAY_CHANNELS;

    Integration<PDU_TRANSPORT_CHANNELS, CAN_CHANNELS, FR_CHANNELS> integration;

    integration.init(
        ::blob::CONFIGURATION_BLOB,
        {},
        {},
        ::routing::canChannelIds,
        _canReaders,
        _canWriters,
        ::routing::frChannelIds,
        _frReaders,
        _frWriters,
        {});

    EXPECT_EQ(PDU_TRANSPORT_CHANNELS, integration.pduTransportChannelAdapters().size());
    EXPECT_EQ(CAN_CHANNELS, integration.canChannelAdapters().size());
    EXPECT_EQ(FR_CHANNELS, integration.flexrayChannelAdapters().size());

    EXPECT_THAT(
        channelIds<NUM_CAN_CHANNELS>(integration.canChannelAdapters()),
        ElementsAreArray(::routing::canChannelIds));
    EXPECT_THAT(
        channelIds<NUM_FLEXRAY_CHANNELS>(integration.flexrayChannelAdapters()),
        ElementsAreArray(::routing::frChannelIds));

    writePdu(3, 16, _canInputWriters.front());
    writePdu(18, 16, _pduTransportInputWriters.front());
    writePdu(3435969450, 72, _frInputWriters.front());

    integration.route();
}

/**
 * \desc: Initializing integration without CAN channels doesn't crash.
 */
TEST_F(IntegrationTest, no_can_channels)
{
    constexpr auto PDU_TRANSPORT_CHANNELS = NUM_PDU_TRANSPORT_CHANNELS;
    constexpr size_t CAN_CHANNELS         = 0;
    constexpr auto FR_CHANNELS            = NUM_FLEXRAY_CHANNELS;

    Integration<PDU_TRANSPORT_CHANNELS, CAN_CHANNELS, FR_CHANNELS> integration;

    integration.init(
        ::blob::CONFIGURATION_BLOB,
        ::etl::make_span(_pduTransportReaders),
        ::etl::make_span(_pduTransportWriters),
        {},
        {},
        {},
        ::routing::frChannelIds,
        _frReaders,
        _frWriters,
        {});

    EXPECT_EQ(
        PDU_TRANSPORT_CHANNELS - NUM_CAN_CHANNELS,
        integration.pduTransportChannelAdapters().size());
    EXPECT_EQ(CAN_CHANNELS, integration.canChannelAdapters().size());
    EXPECT_EQ(FR_CHANNELS, integration.flexrayChannelAdapters().size());

    EXPECT_THAT(
        channelIds<PDU_TRANSPORT_CHANNELS - NUM_CAN_CHANNELS>(
            integration.pduTransportChannelAdapters()),
        ElementsAreArray(::etl::make_span(_pduTransportChannelIds)
                             .first(PDU_TRANSPORT_CHANNELS - NUM_CAN_CHANNELS)));
    EXPECT_THAT(
        channelIds<NUM_FLEXRAY_CHANNELS>(integration.flexrayChannelAdapters()),
        ElementsAreArray(::routing::frChannelIds));

    writePdu(3, 16, _canInputWriters.front());
    writePdu(18, 16, _pduTransportInputWriters.front());
    writePdu(3435969450, 72, _frInputWriters.front());

    integration.route();
}

/**
 * \desc: Initializing integration without Flexray channels doesn't crash.
 */
TEST_F(IntegrationTest, no_fr_channels)
{
    constexpr auto PDU_TRANSPORT_CHANNELS = NUM_PDU_TRANSPORT_CHANNELS;
    constexpr auto CAN_CHANNELS           = NUM_CAN_CHANNELS;
    constexpr size_t FR_CHANNELS          = 0;

    Integration<PDU_TRANSPORT_CHANNELS, CAN_CHANNELS, FR_CHANNELS> integration;

    integration.init(
        ::blob::CONFIGURATION_BLOB,
        ::etl::make_span(_pduTransportReaders),
        ::etl::make_span(_pduTransportWriters),
        ::routing::canChannelIds,
        _canReaders,
        _canWriters,
        {},
        {},
        {},
        {});

    EXPECT_EQ(
        PDU_TRANSPORT_CHANNELS - NUM_FLEXRAY_CHANNELS,
        integration.pduTransportChannelAdapters().size());
    EXPECT_EQ(CAN_CHANNELS, integration.canChannelAdapters().size());
    EXPECT_EQ(FR_CHANNELS, integration.flexrayChannelAdapters().size());

    EXPECT_THAT(
        channelIds<PDU_TRANSPORT_CHANNELS - NUM_FLEXRAY_CHANNELS>(
            integration.pduTransportChannelAdapters()),
        ElementsAreArray(::etl::make_span(_pduTransportChannelIds)
                             .first(PDU_TRANSPORT_CHANNELS - NUM_FLEXRAY_CHANNELS)));
    EXPECT_THAT(
        channelIds<NUM_CAN_CHANNELS>(integration.canChannelAdapters()),
        ElementsAreArray(::routing::canChannelIds));

    writePdu(3, 16, _canInputWriters.front());
    writePdu(18, 16, _pduTransportInputWriters.front());
    writePdu(3435969450, 72, _frInputWriters.front());

    integration.route();
}

/**
 * \desc: Adapters with invalid readers or writers are not created.
 */
TEST_F(IntegrationTest, invalid_readers_or_writers)
{
    auto pduTransportReaders    = _pduTransportReaders;
    auto pduTransportWriters    = _pduTransportWriters;
    pduTransportReaders.front() = nullptr;
    pduTransportWriters.back()  = nullptr;

    auto canReaders    = _canReaders;
    auto canWriters    = _canWriters;
    canReaders.front() = nullptr;
    canWriters.back()  = nullptr;

    auto frReaders    = _frReaders;
    auto frWriters    = _frWriters;
    frReaders.front() = nullptr;

    Integration<> integration;

    integration.init(
        ::blob::CONFIGURATION_BLOB,
        ::etl::make_span(pduTransportReaders),
        ::etl::make_span(pduTransportWriters),
        ::routing::canChannelIds,
        canReaders,
        canWriters,
        ::routing::frChannelIds,
        frReaders,
        frWriters,
        {});

    EXPECT_EQ(NUM_PDU_TRANSPORT_CHANNELS - 2, integration.pduTransportChannelAdapters().size());
    EXPECT_EQ(NUM_CAN_CHANNELS - 2, integration.canChannelAdapters().size());
    EXPECT_EQ(NUM_FLEXRAY_CHANNELS - 1, integration.flexrayChannelAdapters().size());

    EXPECT_THAT(
        channelIds<NUM_PDU_TRANSPORT_CHANNELS - 2>(integration.pduTransportChannelAdapters()),
        ElementsAreArray(
            ::etl::make_span(_pduTransportChannelIds).subspan(1, NUM_PDU_TRANSPORT_CHANNELS - 2)));

    new (&integration) Integration<>();

    frReaders.front() = _frReaders.front();
    frWriters.front() = nullptr;

    integration.init(
        ::blob::CONFIGURATION_BLOB,
        ::etl::make_span(pduTransportReaders),
        ::etl::make_span(pduTransportWriters),
        ::routing::canChannelIds,
        canReaders,
        canWriters,
        ::routing::frChannelIds,
        frReaders,
        frWriters,
        {});

    EXPECT_EQ(NUM_PDU_TRANSPORT_CHANNELS - 2, integration.pduTransportChannelAdapters().size());
    EXPECT_EQ(NUM_CAN_CHANNELS - 2, integration.canChannelAdapters().size());
    EXPECT_EQ(NUM_FLEXRAY_CHANNELS - 1, integration.flexrayChannelAdapters().size());

    EXPECT_THAT(
        channelIds<NUM_PDU_TRANSPORT_CHANNELS - 2>(integration.pduTransportChannelAdapters()),
        ElementsAreArray(
            ::etl::make_span(_pduTransportChannelIds).subspan(1, NUM_PDU_TRANSPORT_CHANNELS - 2)));
}

/**
 * \desc: Reinitializing integration has no effect.
 */
TEST_F(IntegrationTest, reinit)
{
    Integration<> integration;

    integration.init(
        ::blob::CONFIGURATION_BLOB,
        ::etl::make_span(_pduTransportReaders),
        ::etl::make_span(_pduTransportWriters),
        ::routing::canChannelIds,
        _canReaders,
        _canWriters,
        ::routing::frChannelIds,
        _frReaders,
        _frWriters,
        {});

    EXPECT_EQ(NUM_PDU_TRANSPORT_CHANNELS, integration.pduTransportChannelAdapters().size());
    EXPECT_EQ(NUM_CAN_CHANNELS, integration.canChannelAdapters().size());
    EXPECT_EQ(NUM_FLEXRAY_CHANNELS, integration.flexrayChannelAdapters().size());

    EXPECT_THAT(
        channelIds<NUM_CAN_CHANNELS>(integration.canChannelAdapters()),
        ElementsAreArray(::routing::canChannelIds));
    EXPECT_THAT(
        channelIds<NUM_FLEXRAY_CHANNELS>(integration.flexrayChannelAdapters()),
        ElementsAreArray(::routing::frChannelIds));
    EXPECT_THAT(
        channelIds<NUM_PDU_TRANSPORT_CHANNELS>(integration.pduTransportChannelAdapters()),
        ElementsAreArray(_pduTransportChannelIds));

    _pduTransportReaders.fill(nullptr);
    _pduTransportWriters.fill(nullptr);
    _canReaders.fill(nullptr);
    _canWriters.fill(nullptr);
    _frReaders.fill(nullptr);
    _frWriters.fill(nullptr);

    integration.init(
        ::blob::CONFIGURATION_BLOB,
        ::etl::make_span(_pduTransportReaders),
        ::etl::make_span(_pduTransportWriters),
        ::routing::canChannelIds,
        _canReaders,
        _canWriters,
        ::routing::frChannelIds,
        _frReaders,
        _frWriters,
        {});

    EXPECT_EQ(NUM_PDU_TRANSPORT_CHANNELS, integration.pduTransportChannelAdapters().size());
    EXPECT_EQ(NUM_CAN_CHANNELS, integration.canChannelAdapters().size());
    EXPECT_EQ(NUM_FLEXRAY_CHANNELS, integration.flexrayChannelAdapters().size());

    EXPECT_THAT(
        channelIds<NUM_CAN_CHANNELS>(integration.canChannelAdapters()),
        ElementsAreArray(::routing::canChannelIds));
    EXPECT_THAT(
        channelIds<NUM_FLEXRAY_CHANNELS>(integration.flexrayChannelAdapters()),
        ElementsAreArray(::routing::frChannelIds));
    EXPECT_THAT(
        channelIds<NUM_PDU_TRANSPORT_CHANNELS>(integration.pduTransportChannelAdapters()),
        ElementsAreArray(_pduTransportChannelIds));
}

/**
 * \desc: Routing somo PDUs from/to different channels works.
 */
TEST_F(IntegrationTest, route_pdus)
{
    Integration<> integration;

    integration.init(
        ::blob::CONFIGURATION_BLOB,
        ::etl::make_span(_pduTransportReaders),
        ::etl::make_span(_pduTransportWriters),
        ::routing::canChannelIds,
        _canReaders,
        _canWriters,
        ::routing::frChannelIds,
        _frReaders,
        _frWriters,
        {});

    auto& canInputWriter = _canInputWriters.front();
    writePdu(3, 16, canInputWriter);
    writePdu(3, 16, canInputWriter);

    auto& pduTransportInputWriter = _pduTransportInputWriters.front();
    writePdu(17, 16, pduTransportInputWriter);
    writePdu(17, 16, pduTransportInputWriter);

    auto& frInputWriter = _frInputWriters.front();
    writePdu(3435969450, 72, frInputWriter);
    writePdu(3435969450, 72, frInputWriter);

    auto& canInputReader           = _canInputReaders.front();
    auto& pduTransportInputReader  = _pduTransportInputReaders.front();
    auto& frInputReader            = _frInputReaders.front();
    auto& pduTransportOutputReader = _pduTransportOutputReaders.front();
    auto& canOutputReader          = _canOutputReaders.front();
    auto& frOutputReader           = _frOutputReaders.front();

    EXPECT_EQ(16, canInputReader.peek().size());
    EXPECT_EQ(16, pduTransportInputReader.peek().size());
    EXPECT_EQ(72, frInputReader.peek().size());
    EXPECT_EQ(0, pduTransportOutputReader.peek().size());
    EXPECT_EQ(0, canOutputReader.peek().size());
    EXPECT_EQ(0, frOutputReader.peek().size());

    EXPECT_TRUE(integration.route());
    EXPECT_TRUE(integration.route());
    EXPECT_TRUE(integration.route());

    EXPECT_EQ(16, canInputReader.peek().size());
    EXPECT_EQ(16, pduTransportInputReader.peek().size());
    EXPECT_EQ(72, frInputReader.peek().size());
    EXPECT_EQ(72, pduTransportOutputReader.peek().size());
    EXPECT_EQ(16, canOutputReader.peek().size());
    EXPECT_EQ(16, frOutputReader.peek().size());

    pduTransportOutputReader.release();
    canOutputReader.release();
    frOutputReader.release();

    EXPECT_EQ(0, pduTransportOutputReader.peek().size());
    EXPECT_EQ(0, canOutputReader.peek().size());
    EXPECT_EQ(0, frOutputReader.peek().size());

    EXPECT_TRUE(integration.route());
    EXPECT_TRUE(integration.route());
    EXPECT_TRUE(integration.route());

    EXPECT_EQ(0, canInputReader.peek().size());
    EXPECT_EQ(0, pduTransportInputReader.peek().size());
    EXPECT_EQ(0, frInputReader.peek().size());
    EXPECT_EQ(72, pduTransportOutputReader.peek().size());
    EXPECT_EQ(16, canOutputReader.peek().size());
    EXPECT_EQ(16, frOutputReader.peek().size());

    pduTransportOutputReader.release();
    canOutputReader.release();
    frOutputReader.release();

    EXPECT_EQ(0, pduTransportOutputReader.peek().size());
    EXPECT_EQ(0, canOutputReader.peek().size());
    EXPECT_EQ(0, frOutputReader.peek().size());
}

/**
 * \desc: Routing some PDUs from/to different channels works with a smaller number of
 * channels.
 */
TEST_F(IntegrationTest, route_pdus_smaller_number_of_pdu_transport_channels)
{
    constexpr auto PDU_TRANSPORT_CHANNELS = NUM_PDU_TRANSPORT_CHANNELS - 1;

    Integration<> integration;

    integration.init(
        ::blob::CONFIGURATION_BLOB,
        ::etl::make_span(_pduTransportReaders).first(PDU_TRANSPORT_CHANNELS),
        ::etl::make_span(_pduTransportWriters).first(PDU_TRANSPORT_CHANNELS),
        ::routing::canChannelIds,
        _canReaders,
        _canWriters,
        ::routing::frChannelIds,
        _frReaders,
        _frWriters,
        {});

    writePdu(3, 16, _canInputWriters.front());
    writePdu(3, 16, _canInputWriters.front());
    writePdu(17, 16, _pduTransportInputWriters.front());
    writePdu(17, 16, _pduTransportInputWriters.front());
    writePdu(3435969450, 72, _frInputWriters.front());
    writePdu(3435969450, 72, _frInputWriters.front());

    EXPECT_TRUE(integration.route());
    EXPECT_TRUE(integration.route());
    EXPECT_TRUE(integration.route());

    EXPECT_EQ(72, _pduTransportOutputReaders.front().peek().size());
    EXPECT_EQ(16, _canOutputReaders.front().peek().size());
    EXPECT_EQ(16, _frOutputReaders.front().peek().size());

    _pduTransportOutputReaders.front().release();
    _canOutputReaders.front().release();
    _frOutputReaders.front().release();

    EXPECT_EQ(0, _pduTransportOutputReaders.front().peek().size());
    EXPECT_EQ(0, _canOutputReaders.front().peek().size());
    EXPECT_EQ(0, _frOutputReaders.front().peek().size());

    EXPECT_TRUE(integration.route());
    EXPECT_TRUE(integration.route());
    EXPECT_TRUE(integration.route());

    EXPECT_EQ(72, _pduTransportOutputReaders.front().peek().size());
    EXPECT_EQ(16, _canOutputReaders.front().peek().size());
    EXPECT_EQ(16, _frOutputReaders.front().peek().size());

    _pduTransportOutputReaders.front().release();
    _canOutputReaders.front().release();
    _frOutputReaders.front().release();

    EXPECT_EQ(0, _pduTransportOutputReaders.front().peek().size());
    EXPECT_EQ(0, _canOutputReaders.front().peek().size());
    EXPECT_EQ(0, _frOutputReaders.front().peek().size());
}

/**
 * \desc: Routing some PDUs from/to different channels works with a bigger number of
 * channels.
 */
TEST_F(IntegrationTest, route_pdus_bigger_number_of_pdu_transport_channels)
{
    constexpr auto PDU_TRANSPORT_CHANNELS = NUM_PDU_TRANSPORT_CHANNELS + 1;

    ::etl::array<io::IReader*, PDU_TRANSPORT_CHANNELS> pduTransportReaders;
    ::etl::array<io::IWriter*, PDU_TRANSPORT_CHANNELS> pduTransportWriters;

    for (size_t i = 0; i < NUM_PDU_TRANSPORT_CHANNELS; ++i)
    {
        pduTransportReaders[i] = _pduTransportReaders[i];
        pduTransportWriters[i] = _pduTransportWriters[i];
    }
    pduTransportReaders.back() = nullptr;
    pduTransportWriters.back() = nullptr;

    Integration<> integration;

    integration.init(
        ::blob::CONFIGURATION_BLOB,
        ::etl::make_span(pduTransportReaders),
        ::etl::make_span(pduTransportWriters),
        ::routing::canChannelIds,
        _canReaders,
        _canWriters,
        ::routing::frChannelIds,
        _frReaders,
        _frWriters,
        {});

    writePdu(3, 16, _canInputWriters.front());
    writePdu(3, 16, _canInputWriters.front());
    writePdu(17, 16, _pduTransportInputWriters.front());
    writePdu(17, 16, _pduTransportInputWriters.front());
    writePdu(3435969450, 72, _frInputWriters.front());
    writePdu(3435969450, 72, _frInputWriters.front());

    EXPECT_TRUE(integration.route());
    EXPECT_TRUE(integration.route());
    EXPECT_TRUE(integration.route());

    EXPECT_EQ(72, _pduTransportOutputReaders.front().peek().size());
    EXPECT_EQ(16, _canOutputReaders.front().peek().size());
    EXPECT_EQ(16, _frOutputReaders.front().peek().size());

    _pduTransportOutputReaders.front().release();
    _canOutputReaders.front().release();
    _frOutputReaders.front().release();

    EXPECT_EQ(0, _pduTransportOutputReaders.front().peek().size());
    EXPECT_EQ(0, _canOutputReaders.front().peek().size());
    EXPECT_EQ(0, _frOutputReaders.front().peek().size());

    EXPECT_TRUE(integration.route());
    EXPECT_TRUE(integration.route());
    EXPECT_TRUE(integration.route());

    EXPECT_EQ(72, _pduTransportOutputReaders.front().peek().size());
    EXPECT_EQ(16, _canOutputReaders.front().peek().size());
    EXPECT_EQ(16, _frOutputReaders.front().peek().size());

    _pduTransportOutputReaders.front().release();
    _canOutputReaders.front().release();
    _frOutputReaders.front().release();

    EXPECT_EQ(0, _pduTransportOutputReaders.front().peek().size());
    EXPECT_EQ(0, _canOutputReaders.front().peek().size());
    EXPECT_EQ(0, _frOutputReaders.front().peek().size());
}

} // namespace
