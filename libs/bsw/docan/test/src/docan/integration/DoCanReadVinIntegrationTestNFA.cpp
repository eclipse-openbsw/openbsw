/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

// This is an example-style integration test showing how to wire up a
// DoCanTransportLayer with ISO 15765-2 normal fixed addressing (NFA). With
// normal fixed addressing, the complete network address information -
// addressing format (physical/functional), target and source address - is
// encoded directly into a 29 bit (extended) CAN identifier; no address
// extension payload byte is needed (unlike ISO 15765-2 extended addressing,
// see DoCanReadVinIntegrationTestEA.cpp). Since the target/source addresses
// are extracted directly from the CAN identifier, no lookup table is
// required either; this also means the addressing does not need to know its
// "own" address, making it equally suitable for a gateway routing between
// buses.
//
// This example uses physical addressing, source address (N_SA) 0xF1 for the
// tester and target address (N_TA) 0x2A for the ECU (the address used for
// the ECU in referenceApp), resulting in the request CAN identifier
// 0x18DA2AF1 and the (swapped, since responses are always sent physically)
// response CAN identifier 0x18DAF12A. It also configures the standard
// OBD/UDS functional (broadcast) address 0x33 as a functional address, even
// though this particular exchange does not use it.
//
// As with the other DoCanReadVinIntegrationTest* examples, it exercises a
// realistic UDS exchange - ReadDataByIdentifier for the VIN (DID 0xF190) -
// where the request fits into a single CAN frame but the positive response
// is long enough to require a segmented (multi-frame) transmission,
// including handling of the flow control frame sent back by the tester.

#include "docan/addressing/DoCanNormalFixedAddressing.h"
#include "docan/addressing/DoCanNormalFixedAddressingFilter.h"
#include "docan/can/DoCanPhysicalCanTransceiver.h"
#include "docan/common/DoCanParameters.h"
#include "docan/datalink/DoCanDefaultFrameSizeMapper.h"
#include "docan/datalink/DoCanFrameCodecConfigPresets.h"
#include "docan/transmitter/IDoCanTickGenerator.h"
#include "docan/transport/DoCanTransportLayer.h"
#include "docan/transport/DoCanTransportLayerConfig.h"

#include <async/AsyncMock.h>
#include <async/TestContext.h>
#include <can/canframes/CanId.h>
#include <can/transceiver/ICanTransceiverMock.h>
#include <etl/delegate.h>
#include <etl/span.h>
#include <transport/BufferedTransportMessage.h>
#include <transport/TransportMessage.h>
#include <transport/TransportMessageProcessedListenerMock.h>
#include <transport/TransportMessageProvidingListenerMock.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <vector>

namespace
{
using namespace ::testing;
using namespace ::docan;
using namespace ::transport;

// UDS service ids and DID used for this example: ReadDataByIdentifier (0x22/0x62) for the VIN
// (DID 0xF190).
constexpr uint8_t SID_READ_DATA_BY_IDENTIFIER          = 0x22U;
constexpr uint8_t SID_READ_DATA_BY_IDENTIFIER_RESPONSE = 0x62U;
constexpr uint8_t DID_VIN_HIGH                         = 0xF1U;
constexpr uint8_t DID_VIN_LOW                          = 0x90U;
// 17 character VIN, as mandated for ISO 3779/UDS DID 0xF190.
uint8_t const VIN[]
    = {'W', 'V', 'W', 'Z', 'Z', 'Z', '1', 'J', 'Z', 'X', 'W', '0', '0', '0', '0', '0', '1'};

// N_SA of the tester and N_TA of the ECU (the address used for the ECU in referenceApp).
constexpr uint16_t TESTER_ID = 0xF1U;
constexpr uint16_t ECU_ID    = 0x2AU;

// standard OBD/UDS functional (broadcast) address, configured as a functional address for this
// example even though it is not exercised by this particular (physical) request/response.
constexpr uint8_t FUNCTIONAL_ADDRESS = 0x33U;

uint32_t systemMicro() { return 0U; }

::etl::span<uint8_t const> payloadOf(::can::CANFrame const& frame)
{
    return ::etl::span<uint8_t const>(frame.getPayload(), frame.getPayloadLength());
}

// AsyncMock derives from etl::singleton_base, which only allows a single instance to exist per
// process (a second construction throws). Other tests linked into this binary (e.g. DemoTest)
// may have already created and intentionally leaked an AsyncMock instance for the same reason, so
// only create one here if none exists yet, and never destroy it.
alignas(::testing::NiceMock<::async::AsyncMock>) uint8_t
    asyncMockMem[sizeof(::testing::NiceMock<::async::AsyncMock>)];

TEST(DoCanReadVinIntegrationTestNFA, SingleFrameRequestYieldsSegmentedResponse)
{
    if (!::async::AsyncMock::is_valid())
    {
        new (asyncMockMem)::testing::NiceMock<::async::AsyncMock>();
        ::testing::Mock::AllowLeak(asyncMockMem);
    }

    ::async::TestContext context(1);
    context.handleExecute();

    // EXAMPLE_START DoCanNormalFixedAddressing
    using Addressing    = DoCanNormalFixedAddressing<>;
    using DataLinkLayer = Addressing::DataLinkLayerType;
    Addressing addressing;
    // EXAMPLE_END DoCanNormalFixedAddressing

    DoCanParameters const parameters(
        ::etl::delegate<decltype(systemMicro)>::create<&systemMicro>(),
        1000U, // allocate timeout
        1000U, // rx timeout
        1000U, // tx callback timeout
        1000U, // flow control timeout
        1U,    // allocate retry count
        1U,    // flow control wait count
        0U,    // min separation time
        0U);   // block size: request all remaining frames after the first flow control

    using FrameCodecType = DoCanFrameCodec<DataLinkLayer>;
    using MapperType     = DoCanDefaultFrameSizeMapper<DataLinkLayer::FrameSizeType>;

    MapperType const mapper;
    FrameCodecType const codec(DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC, mapper);

    // EXAMPLE_START DoCanNormalFixedAddressingFilter
    // no lookup table is required: source/target address bytes are extracted directly from the
    // CAN identifier of a received frame. Only the (single, shared) codec, the addresses valid
    // for functional addressing and the testers allowed to reach the ECU need to be configured.
    uint8_t const functionalAddresses[] = {FUNCTIONAL_ADDRESS};
    // testers allowed to reach the ECU; a request whose source address is not listed here is
    // dropped silently before UDS processing starts.
    uint8_t const allowedTesters[]      = {static_cast<uint8_t>(TESTER_ID)};
    DoCanNormalFixedAddressingFilter<DataLinkLayer> addressingFilter{
        ::etl::span<uint8_t const>(functionalAddresses),
        ::etl::span<uint8_t const>(allowedTesters),
        codec};
    // EXAMPLE_END DoCanNormalFixedAddressingFilter

    uint8_t const busId = 1U;

    NiceMock<::can::ICanTransceiverMock> canTransceiver;
    std::vector<::can::CANFrame> sentFrames;
    // real hardware sends asynchronously and notifies the sent-listener later on (typically from
    // ISR context) - defer the notification until flushPendingSentNotification() is called below,
    // to mirror that timing instead of notifying reentrantly while write() is still on the stack.
    ::can::CANFrame pendingSentFrame;
    ::can::ICANFrameSentListener* pendingSentListener = nullptr;
    ON_CALL(canTransceiver, write(_, _))
        .WillByDefault(Invoke(
            [&](::can::CANFrame const& frame, ::can::ICANFrameSentListener& listener)
            {
                sentFrames.push_back(frame);
                pendingSentFrame    = frame;
                pendingSentListener = &listener;
                return ::can::ICanTransceiver::ErrorCode::CAN_ERR_OK;
            }));
    auto const flushPendingSentNotifications = [&]()
    {
        while (pendingSentListener != nullptr)
        {
            ::can::ICANFrameSentListener* const listener = pendingSentListener;
            ::can::CANFrame const frame                  = pendingSentFrame;
            pendingSentListener                          = nullptr;
            listener->canFrameSent(frame);
        }
    };

    // EXAMPLE_START DoCanPhysicalCanTransceiver
    DoCanPhysicalCanTransceiver<Addressing> transceiver(
        canTransceiver, addressingFilter, addressingFilter, addressing);

    // EXAMPLE_END DoCanPhysicalCanTransceiver

    struct TickGenerator : IDoCanTickGenerator
    {
        void tickNeeded() override {}
    };

    TickGenerator tickGenerator;

    ::docan::declare::DoCanTransportLayerConfig<DataLinkLayer, 1U, 1U, 8U> transportLayerConfig(
        parameters);

    uint8_t const loggerComponent = 0U;
    // EXAMPLE_START DoCanTransportLayer
    DoCanTransportLayer<DataLinkLayer> transportLayer(
        busId,
        context,
        addressingFilter,
        transceiver,
        tickGenerator,
        transportLayerConfig,
        loggerComponent);
    // EXAMPLE_END DoCanTransportLayer

    NiceMock<TransportMessageProvidingListenerMock> messageProvidingListenerMock;
    transportLayer.fProvidingListenerHelper.fpMessageProvider = &messageProvidingListenerMock;
    transportLayer.fProvidingListenerHelper.fpMessageListener = &messageProvidingListenerMock;

    NiceMock<TransportMessageProcessedListenerMock> responseProcessedListenerMock;

    ASSERT_EQ(::transport::AbstractTransportLayer::ErrorCode::TP_OK, transportLayer.init());

    // The response message must outlive the send() call below (transmission finishes
    // synchronously in this test setup, but keep it in the test scope for clarity).
    BufferedTransportMessage<32U> responseMessage;

    // EXAMPLE_START ReceiveRequestAndSendSegmentedResponse
    EXPECT_CALL(messageProvidingListenerMock, messageReceived(busId, _, _))
        .WillOnce(Invoke(
            [&](uint8_t,
                TransportMessage& requestMessage,
                ITransportMessageProcessedListener* processedListener)
                -> ITransportMessageListener::ReceiveResult
            {
                // verify the single-frame UDS request: ReadDataByIdentifier(VIN)
                EXPECT_EQ(TESTER_ID, requestMessage.getSourceId());
                EXPECT_EQ(ECU_ID, requestMessage.getTargetId());
                EXPECT_EQ(3U, requestMessage.getPayloadLength());
                EXPECT_EQ(SID_READ_DATA_BY_IDENTIFIER, requestMessage.getPayload()[0]);
                EXPECT_EQ(DID_VIN_HIGH, requestMessage.getPayload()[1]);
                EXPECT_EQ(DID_VIN_LOW, requestMessage.getPayload()[2]);

                // build and send the (segmented) positive response
                responseMessage.setSourceAddress(ECU_ID);
                responseMessage.setTargetAddress(TESTER_ID);
                responseMessage.setPayloadLength(3U + sizeof(VIN));
                (void)responseMessage.append(SID_READ_DATA_BY_IDENTIFIER_RESPONSE);
                (void)responseMessage.append(DID_VIN_HIGH);
                (void)responseMessage.append(DID_VIN_LOW);
                (void)responseMessage.append(VIN, sizeof(VIN));

                EXPECT_EQ(
                    ::transport::AbstractTransportLayer::ErrorCode::TP_OK,
                    transportLayer.send(responseMessage, &responseProcessedListenerMock));

                if (processedListener != nullptr)
                {
                    processedListener->transportMessageProcessed(
                        requestMessage,
                        ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR);
                }
                return ITransportMessageListener::ReceiveResult::RECEIVED_NO_ERROR;
            }));

    // simulate reception of the single-frame UDS request "22 F1 90" physically addressed from the
    // tester (N_SA 0xF1) to the ECU (N_TA 0x2A), i.e. CAN identifier 0x18DA2AF1.
    {
        uint8_t requestPayload[8] = {};
        ::etl::span<uint8_t> payload(requestPayload);
        uint8_t const requestData[] = {SID_READ_DATA_BY_IDENTIFIER, DID_VIN_HIGH, DID_VIN_LOW};
        DataLinkLayer::FrameSizeType consumedDataSize = 0U;
        ASSERT_EQ(
            CodecResult::OK,
            codec.encodeDataFrame(
                payload, ::etl::span<uint8_t const>(requestData), 0U, 0U, consumedDataSize));
        uint32_t const requestCanId = Addressing::pack(
            Addressing::PHYSICAL_ADDRESSING_ID_BASE,
            static_cast<uint8_t>(ECU_ID),
            static_cast<uint8_t>(TESTER_ID));
        ::can::CANFrame const requestFrame(requestCanId, payload.data(), payload.size());
        transceiver.getListener().frameReceived(requestFrame);
    }

    // drive reception processing and the resulting send() of the segmented response until the
    // first frame has been transmitted; then simulate the transceiver notifying us that the
    // first frame has actually gone out on the bus.
    context.execute();
    flushPendingSentNotifications();

    // simulate the flow control frame sent back by the tester ("clear to send", no further
    // flow control needed since block size 0 means "send all remaining frames"), physically
    // addressed the same way as the request.
    {
        uint8_t flowControlPayload[8] = {};
        ::etl::span<uint8_t> payload(flowControlPayload);
        ASSERT_EQ(CodecResult::OK, codec.encodeFlowControlFrame(payload, FlowStatus::CTS, 0U, 0U));
        uint32_t const requestCanId = Addressing::pack(
            Addressing::PHYSICAL_ADDRESSING_ID_BASE,
            static_cast<uint8_t>(ECU_ID),
            static_cast<uint8_t>(TESTER_ID));
        ::can::CANFrame const flowControlFrame(requestCanId, payload.data(), payload.size());
        transceiver.getListener().frameReceived(flowControlFrame);
    }
    // the remaining consecutive frames are sent back-to-back (block size 0, min separation time
    // 0); flushing repeatedly simulates the transceiver confirming each of them as they go out.
    flushPendingSentNotifications();
    context.execute();
    // EXAMPLE_END ReceiveRequestAndSendSegmentedResponse

    // verify that a first frame followed by consecutive frames were sent to the tester with the
    // expected (swapped, physical) response CAN identifier, and that they reassemble into the
    // expected positive VIN response.
    uint32_t const responseCanId = Addressing::pack(
        Addressing::PHYSICAL_ADDRESSING_ID_BASE,
        static_cast<uint8_t>(TESTER_ID),
        static_cast<uint8_t>(ECU_ID));
    ASSERT_GT(sentFrames.size(), 1U);
    for (::can::CANFrame const& frame : sentFrames)
    {
        EXPECT_EQ(responseCanId, frame.getId());
    }

    DataLinkLayer::MessageSizeType messageSize{};
    DataLinkLayer::FrameIndexType frameCount{};
    DataLinkLayer::FrameSizeType consecutiveFrameDataSize{};
    ::etl::span<uint8_t const> data;
    ASSERT_EQ(
        CodecResult::OK,
        codec.decodeFirstFrame(
            payloadOf(sentFrames[0]), messageSize, frameCount, consecutiveFrameDataSize, data));
    EXPECT_EQ(20U, messageSize);
    EXPECT_EQ(sentFrames.size(), frameCount);
    std::vector<uint8_t> reassembled(data.begin(), data.end());

    for (uint8_t expectedSequenceNumber = 1U; expectedSequenceNumber < frameCount;
         ++expectedSequenceNumber)
    {
        uint8_t sequenceNumber{};
        ASSERT_EQ(
            CodecResult::OK,
            codec.decodeConsecutiveFrame(
                payloadOf(sentFrames[expectedSequenceNumber]), sequenceNumber, data));
        EXPECT_EQ(expectedSequenceNumber, sequenceNumber);
        reassembled.insert(reassembled.end(), data.begin(), data.end());
    }

    std::vector<uint8_t> expectedResponse{
        SID_READ_DATA_BY_IDENTIFIER_RESPONSE, DID_VIN_HIGH, DID_VIN_LOW};
    expectedResponse.insert(expectedResponse.end(), std::begin(VIN), std::end(VIN));
    EXPECT_EQ(expectedResponse, reassembled);
}

} // namespace
