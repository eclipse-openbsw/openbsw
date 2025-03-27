// Copyright 2024 Accenture.

#pragma once

#include "docan/common/DoCanConnection.h"
#include "docan/common/DoCanConstants.h"
#include "docan/datalink/DoCanDataLinkLayer.h"
#include "docan/datalink/DoCanFrameCodec.h"
#include "docan/datalink/IDoCanFrameReceiver.h"

#include <estd/algorithm.h>
#include <estd/memory.h>
#include <estd/slice.h>

namespace docan
{
/**
 * helper class for encoding/decoding frame payloads according to DoCAN spec.
 * \tparam DataLinkLayer class providing data link functionality
 */
template<class DataLinkLayer>
class DoCanFrameDecoder
{
public:
    using DataLinkLayerType   = DataLinkLayer;
    using CodecType           = DoCanFrameCodec<DataLinkLayerType>;
    using DataLinkAddressType = typename DataLinkLayerType::AddressType;
    using MessageSizeType     = typename DataLinkLayerType::MessageSizeType;
    using FrameIndexType      = typename DataLinkLayerType::FrameIndexType;
    using FrameSizeType       = typename DataLinkLayerType::FrameSizeType;

    /**
     * Decode a frame and send it to frame receiver.
     * \param receptionAddress reception address
     * \param payload reference to frame payload
     * \param receiver reference to receiver of frame
     * \return OK in case of successful extraction and delivery to receiver
     */
    static ::docan::CodecResult decodeFrame(
        DoCanConnection<DataLinkLayerType> const& connection,
        ::estd::slice<uint8_t const> const& payload,
        IDoCanFrameReceiver<DataLinkLayerType>& receiver);
};

template<class Codec>
CodecResult DoCanFrameDecoder<Codec>::decodeFrame(
    DoCanConnection<DataLinkLayerType> const& connection,
    ::estd::slice<uint8_t const> const& payload,
    IDoCanFrameReceiver<DataLinkLayerType>& receiver)
{
    CodecType const& codec = connection.getFrameCodec();
    FrameType frameType;
    CodecResult result = codec.decodeFrameType(payload, frameType);
    if (result == CodecResult::OK)
    {
        switch (frameType)
        {
            case FrameType::SINGLE:
            {
                ::estd::slice<uint8_t const> data;
                MessageSizeType messageSize;
                result = codec.decodeSingleFrame(payload, messageSize, data);
                if (result == CodecResult::OK)
                {
                    receiver.firstDataFrameReceived(connection, messageSize, 1U, 0U, data);
                }
                break;
            }
            case FrameType::FIRST:
            {
                ::estd::slice<uint8_t const> data;
                MessageSizeType messageSize;
                FrameIndexType frameCount;
                FrameSizeType consecutiveDataFrameSize;
                result = codec.decodeFirstFrame(
                    payload, messageSize, frameCount, consecutiveDataFrameSize, data);
                if (result == CodecResult::OK)
                {
                    receiver.firstDataFrameReceived(
                        connection, messageSize, frameCount, consecutiveDataFrameSize, data);
                }
                break;
            }
            case FrameType::CONSECUTIVE:
            {
                ::estd::slice<uint8_t const> data;
                uint8_t sequenceNumber;
                result = codec.decodeConsecutiveFrame(payload, sequenceNumber, data);
                if (result == CodecResult::OK)
                {
                    receiver.consecutiveDataFrameReceived(
                        connection.getDataLinkAddressPair().getReceptionAddress(),
                        sequenceNumber,
                        data);
                }
                break;
            }
            case FrameType::FLOW_CONTROL:
            {
                FlowStatus flowStatus;
                uint8_t blockSize;
                uint8_t encodedMinSeparationTime;
                result = codec.decodeFlowControlFrame(
                    payload, flowStatus, blockSize, encodedMinSeparationTime);
                if (result == CodecResult::OK)
                {
                    receiver.flowControlFrameReceived(
                        connection.getDataLinkAddressPair().getReceptionAddress(),
                        flowStatus,
                        blockSize,
                        encodedMinSeparationTime);
                }
                break;
            }
            default:
            {
                return CodecResult::INVALID_FRAME_TYPE;
            }
        }
    }
    return result;
}

} // namespace docan
