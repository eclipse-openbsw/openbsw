// Copyright 2024 Accenture.

#pragma once

#include "docan/common/DoCanConstants.h"
#include "docan/datalink/DoCanDataLinkLayer.h"
#include "docan/datalink/DoCanFrameCodecConfig.h"
#include "docan/datalink/IDoCanFrameReceiver.h"
#include "docan/datalink/IDoCanFrameSizeMapper.h"

#include <etl/algorithm.h>
#include <etl/limits.h>
#include <etl/unaligned_type.h>
#include <util/estd/assert.h>

#include <cstdint>
#include <limits>

namespace docan
{

/**
 * enumeration defining results of decoding.
 */
enum class CodecResult : uint8_t
{
    /// decoding was successful
    OK,
    /// frame is not valid due to length
    INVALID_FRAME_SIZE,
    /// message size is not valid
    INVALID_MESSAGE_SIZE,
    /// the frame index is not valid
    INVALID_FRAME_INDEX,
    /// the frame type is not known
    INVALID_FRAME_TYPE
};

static constexpr uint8_t EXTENDED_SF_DL_EDGE_SIZE  = 8U;
static constexpr uint16_t ESCAPED_SEQ_MESSAGE_SIZE = 4095U;

/**
 * helper class for encoding/decoding frame payloads according to DoCAN spec.
 * \tparam DataLinkLayer class providing data link functionality
 */
template<class DataLinkLayer>
class DoCanFrameCodec
{
public:
    using DataLinkLayerType   = DataLinkLayer;
    using DataLinkAddressType = typename DataLinkLayerType::AddressType;
    using MessageSizeType     = typename DataLinkLayerType::MessageSizeType;
    using FrameIndexType      = typename DataLinkLayerType::FrameIndexType;
    using FrameSizeType       = typename DataLinkLayerType::FrameSizeType;

    // Standard define max Escaped seq message size to be uint32_t max.
    static_assert(
        ::etl::numeric_limits<MessageSizeType>::max() <= ::etl::numeric_limits<uint32_t>::max(),
        "MessageSizeType must be a uint32_t");
    static_assert(
        ::etl::numeric_limits<MessageSizeType>::min() >= ::etl::numeric_limits<uint32_t>::min(),
        "MessageSizeType must be a uint32_t");

    /**
     * Default constructor.
     */
    DoCanFrameCodec(
        DoCanFrameCodecConfig<FrameSizeType> const& config,
        IDoCanFrameSizeMapper<FrameSizeType> const& mapper);

    /**
     * Get the frame type from a received payload.
     * \param payload reference to frame payload
     * \param frameType reference to variable that receives frame type
     * \return OK in case of successful extraction
     */
    CodecResult
    decodeFrameType(::etl::span<uint8_t const> const& payload, FrameType& frameType) const;

    /**
     * Extract the data of a single frame.
     * \param payload reference to payload to extract data from and that will receive
     * \param messageSize reference to variable that receives message size
     * \param data reference to slice that receives frame data
     * \return OK in case of successful extraction
     */
    CodecResult decodeSingleFrame(
        ::etl::span<uint8_t const> const& payload,
        MessageSizeType& messageSize,
        ::etl::span<uint8_t const>& data) const;

    /**
     * Extract the data of a first frame.
     * \param payload reference to payload to extract data from and that will receive
     * \param messageSize reference to variable that receives message size
     * \param frameCount reference to variable that receives total number of expected frames
     * \param consecutiveFrameDataSize reference to variable that receives the maximum frame
     *        size of consecutive frames
     * \param data reference to slice that receives frame data
     * \return OK in case of successful extraction
     */
    CodecResult decodeFirstFrame(
        ::etl::span<uint8_t const> const& payload,
        MessageSizeType& messageSize,
        FrameIndexType& frameCount,
        FrameSizeType& consecutiveFrameDataSize,
        ::etl::span<uint8_t const>& data) const;

    /**
     * Extract the data of a consecutive frame.
     * \param payload reference to payload to extract data from and that will receive
     * \param sequenceNumber reference to variable that receives sequence number
     * \param data reference to slice that receives frame data
     * \return OK in case of successful extraction
     */
    CodecResult decodeConsecutiveFrame(
        ::etl::span<uint8_t const> const& payload,
        uint8_t& sequenceNumber,
        ::etl::span<uint8_t const>& data) const;

    /**
     * Extract the data of a flow control frame.
     * \param payload reference to payload to extract data from and that will receive
     * \param flowStatus reference to variable that receives flow status
     * \param blockSize reference to variable that receives block size
     * \param encodedMinSeparationTime reference to variable that receives encoded
     *        minimum separation time
     * \return OK in case of successful extraction
     */
    CodecResult decodeFlowControlFrame(
        ::etl::span<uint8_t const> const& payload,
        FlowStatus& flowStatus,
        uint8_t& blockSize,
        uint8_t& encodedMinSeparationTime) const;

    /**
     * Determine the number of frames needed to send a given message size.
     * \param messageSize message size to get number of frames for
     * \param frameCount reference to variable that receives frame count
     * \param consecutiveFrameDataSize reference to variable that receives consecutive
     *        data frame size
     * \return OK in case message size is valid
     */
    CodecResult getEncodedFrameCount(
        MessageSizeType messageSize,
        FrameIndexType& frameCount,
        FrameSizeType& consecutiveFrameDataSize) const;

    /**
     * Set the payload of a data frame.
     * \param payload reference to payload buffer to encode. The payload will size will be
     *        adjusted if successful
     * \param data reference to data. Contains the remaining message data if successful
     * \param frameIndex index of current frame
     * \param consecutiveFrameDataSize reference to variable that receives consecutive
     *        data frame size
     * \param consumedDataSize reference to variable that receives number of consumed data bytes
     * \return OK in case the frame has been set successfully
     */
    CodecResult encodeDataFrame(
        ::etl::span<uint8_t>& payload,
        ::etl::span<uint8_t const> const& data,
        FrameIndexType frameIndex,
        FrameSizeType consecutiveFrameDataSize,
        FrameSizeType& consumedDataSize) const;

    /**
     * Set the payload of a flow control frame.
     * \param payload reference to payload buffer to encode. The payload will size will be
     *        adjusted if successful
     * \param flowStatus flow status to set
     * \param blockSize block size to set
     * \param encodedMinSeparationTime encoded min separation time to set
     * \return OK in case the frame has been set successfully
     */
    CodecResult encodeFlowControlFrame(
        ::etl::span<uint8_t>& payload,
        FlowStatus flowStatus,
        uint8_t blockSize,
        uint8_t encodedMinSeparationTime) const;

private:
    CodecResult adjustFrame(
        ::etl::span<uint8_t>& payload, FrameSizeType payloadSize, FrameSizeType minFrameSize) const;

    bool checkFrameSize(
        ::etl::span<uint8_t const> const& payload,
        FrameSizeType minPayloadSize,
        typename DoCanFrameCodecConfig<FrameSizeType>::SizeConfig frameSize) const;

    bool fitsIntoShortSingleFrame(MessageSizeType messageSize) const;
    bool fitsIntoLongSingleFrame(MessageSizeType messageSize) const;

    DoCanFrameCodecConfig<FrameSizeType> const& _config;
    IDoCanFrameSizeMapper<FrameSizeType> const& _mapper;
};

/**
 * Inline implementations.
 */
template<class DataLinkLayer>
DoCanFrameCodec<DataLinkLayer>::DoCanFrameCodec(
    DoCanFrameCodecConfig<FrameSizeType> const& config,
    IDoCanFrameSizeMapper<FrameSizeType> const& mapper)
: _config(config), _mapper(mapper)
{}

template<class DataLinkLayer>
CodecResult DoCanFrameCodec<DataLinkLayer>::decodeFrameType(
    ::etl::span<uint8_t const> const& payload, FrameType& frameType) const
{
    uint8_t const offset = _config._offset;
    if (payload.size() <= offset)
    {
        return CodecResult::INVALID_FRAME_SIZE;
    }

    frameType = static_cast<FrameType>((payload[offset] >> 4U) & 0x0FU);
    return CodecResult::OK;
}

template<class DataLinkLayer>
CodecResult DoCanFrameCodec<DataLinkLayer>::decodeSingleFrame(
    ::etl::span<uint8_t const> const& payload,
    MessageSizeType& messageSize,
    ::etl::span<uint8_t const>& data) const
{
    size_t const offset = _config._offset;
    if (checkFrameSize(payload, 2U, _config._singleFrameSize))
    {
        messageSize                       = static_cast<uint8_t>(payload[offset] & 0x0fU);
        bool const isExtendedSfDlExpected = payload.size() > EXTENDED_SF_DL_EDGE_SIZE;
        if ((messageSize == 0U) && isExtendedSfDlExpected)
        {
            messageSize = payload[offset + 1U];
            MessageSizeType const checkPayloadSize
                = messageSize + static_cast<MessageSizeType>(offset) + 2U;

            if ((checkPayloadSize <= static_cast<MessageSizeType>(_config._singleFrameSize._max))
                && (static_cast<size_t>(checkPayloadSize) <= payload.size()))
            {
                data = ::etl::span<uint8_t const>(
                    &payload[offset + 2U], static_cast<size_t>(messageSize));
                return CodecResult::OK;
            }
        }
        else if ((messageSize > 0U) && (!isExtendedSfDlExpected))
        {
            MessageSizeType const checkPayloadSize
                = messageSize + static_cast<MessageSizeType>(offset) + 1U;
            if (checkPayloadSize <= payload.size())
            {
                data = ::etl::span<uint8_t const>(
                    &payload[offset + 1U], static_cast<size_t>(messageSize));
                return CodecResult::OK;
            }
        }
        else
        {
            // invalid message size
        }
        return CodecResult::INVALID_MESSAGE_SIZE;
    }
    return CodecResult::INVALID_FRAME_SIZE;
}

template<class DataLinkLayer>
CodecResult DoCanFrameCodec<DataLinkLayer>::decodeFirstFrame(
    ::etl::span<uint8_t const> const& payload,
    MessageSizeType& messageSize,
    FrameIndexType& frameCount,
    FrameSizeType& consecutiveFrameDataSize,
    ::etl::span<uint8_t const>& data) const
{
    if (checkFrameSize(payload, 3U, _config._firstFrameSize))
    {
        size_t const offset = _config._offset;
        messageSize = static_cast<MessageSizeType>(etl::be_uint16_t(&payload[offset]) & 0xFFFU);
        uint8_t dataStart        = 2U;
        consecutiveFrameDataSize = static_cast<FrameSizeType>(payload.size())
                                   - (static_cast<FrameSizeType>(offset) + 1U);
        if (messageSize == 0U)
        {
            if (!checkFrameSize(payload, 6U, _config._firstFrameSize))
            {
                return CodecResult::INVALID_FRAME_SIZE;
            }
            // Escape sequence, aka message size > 4095 up to uint32_t
            uint32_t const escapedMessageSize = etl::be_uint32_t(&payload[offset + 2U]);

            if ((escapedMessageSize <= ESCAPED_SEQ_MESSAGE_SIZE)
                || (escapedMessageSize > ::etl::numeric_limits<MessageSizeType>::max()))
            {
                return CodecResult::INVALID_MESSAGE_SIZE;
            }
            messageSize = static_cast<MessageSizeType>(escapedMessageSize);
            dataStart   = 6U;
        }

        MessageSizeType const count = (messageSize / consecutiveFrameDataSize);
        if (count >= etl::numeric_limits<FrameIndexType>::max())
        {
            return CodecResult::INVALID_MESSAGE_SIZE;
        }
        frameCount                             = static_cast<FrameIndexType>(count + 1U);
        bool const msgFitsIntoLongSingleFrame  = fitsIntoLongSingleFrame(messageSize);
        bool const msgFitsIntoShortSingleFrame = fitsIntoShortSingleFrame(messageSize);
        if ((frameCount > 1U) && (!msgFitsIntoShortSingleFrame) && (!msgFitsIntoLongSingleFrame))
        {
            data = ::etl::span<uint8_t const>(
                &payload[offset + dataStart],
                static_cast<size_t>(consecutiveFrameDataSize) - dataStart + 1U);
            return CodecResult::OK;
        }
        return CodecResult::INVALID_MESSAGE_SIZE;
    }
    return CodecResult::INVALID_FRAME_SIZE;
}

template<class DataLinkLayer>
CodecResult DoCanFrameCodec<DataLinkLayer>::decodeConsecutiveFrame(
    ::etl::span<uint8_t const> const& payload,
    uint8_t& sequenceNumber,
    ::etl::span<uint8_t const>& data) const
{
    if (checkFrameSize(payload, 2U, _config._consecutiveFrameSize))
    {
        size_t const offset = _config._offset;
        sequenceNumber      = (payload[offset] & 0x0fU);
        data = ::etl::span<uint8_t const>(&payload[offset + 1U], payload.size() - (offset + 1U));
        return CodecResult::OK;
    }
    return CodecResult::INVALID_FRAME_SIZE;
}

template<class DataLinkLayer>
CodecResult DoCanFrameCodec<DataLinkLayer>::decodeFlowControlFrame(
    ::etl::span<uint8_t const> const& payload,
    FlowStatus& flowStatus,
    uint8_t& blockSize,
    uint8_t& encodedMinSeparationTime) const
{
    if (checkFrameSize(payload, 3U, _config._flowControlFrameSize))
    {
        size_t const offset      = _config._offset;
        flowStatus               = static_cast<FlowStatus>(payload[offset] & 0x0fU);
        blockSize                = payload[offset + 1U];
        encodedMinSeparationTime = payload[offset + 2U];
        return CodecResult::OK;
    }
    return CodecResult::INVALID_FRAME_SIZE;
}

template<class DataLinkLayer>
bool DoCanFrameCodec<DataLinkLayer>::fitsIntoShortSingleFrame(
    MessageSizeType const messageSize) const
{
    FrameSizeType const edge = ::etl::min(EXTENDED_SF_DL_EDGE_SIZE, _config._singleFrameSize._max);
    uint8_t const offset     = _config._offset + 1U;
    return messageSize <= static_cast<MessageSizeType>(edge) - offset;
}

template<class DataLinkLayer>
bool DoCanFrameCodec<DataLinkLayer>::fitsIntoLongSingleFrame(
    MessageSizeType const messageSize) const
{
    // Ensure _singleFrameSize._max is at least as large as _config._offset + 2
    estd_assert(_config._singleFrameSize._max >= _config._offset + 2);
    uint8_t const offset = _config._offset + 2U;
    return messageSize <= static_cast<MessageSizeType>(_config._singleFrameSize._max) - offset;
}

template<class DataLinkLayer>
CodecResult DoCanFrameCodec<DataLinkLayer>::getEncodedFrameCount(
    MessageSizeType const messageSize,
    FrameIndexType& frameCount,
    FrameSizeType& consecutiveFrameDataSize) const
{
    // The max consecutive frame size needs to be at least 2 larger than the configured offset for
    // this function.
    estd_assert(_config._consecutiveFrameSize._max >= _config._offset + 2);

    if (messageSize == 0U)
    {
        return CodecResult::INVALID_MESSAGE_SIZE;
    }

    bool const msgFitsIntoLongSingleFrame  = fitsIntoLongSingleFrame(messageSize);
    bool const msgFitsIntoShortSingleFrame = fitsIntoShortSingleFrame(messageSize);
    if (msgFitsIntoShortSingleFrame || msgFitsIntoLongSingleFrame)
    {
        consecutiveFrameDataSize = 0U;
        frameCount               = 1U;
    }
    else
    {
        consecutiveFrameDataSize    = _config._consecutiveFrameSize._max - (_config._offset + 1U);
        MessageSizeType const count = (messageSize / consecutiveFrameDataSize);
        if (count >= etl::numeric_limits<FrameIndexType>::max())
        {
            return CodecResult::INVALID_MESSAGE_SIZE;
        }
        frameCount = static_cast<FrameIndexType>(count + 1U);
    }
    return CodecResult::OK;
}

template<class DataLinkLayer>
CodecResult DoCanFrameCodec<DataLinkLayer>::encodeDataFrame(
    ::etl::span<uint8_t>& payload,
    ::etl::span<uint8_t const> const& data,
    FrameIndexType const frameIndex,
    FrameSizeType const consecutiveFrameDataSize,
    FrameSizeType& consumedDataSize) const
{
    if ((data.size() == 0U) || (data.size() > ::etl::numeric_limits<MessageSizeType>::max()))
    {
        return CodecResult::INVALID_FRAME_INDEX;
    }
    MessageSizeType const pendingMessageSize = static_cast<MessageSizeType>(data.size());

    FrameSizeType destOffset;
    FrameSizeType minFrameSize = 0U;
    if (frameIndex == 0U)
    {
        size_t const offset = _config._offset;
        if (fitsIntoShortSingleFrame(pendingMessageSize))
        {
            payload[offset] = static_cast<uint8_t>(FrameType::SINGLE) << 4U;
            payload[offset] |= static_cast<uint8_t>(pendingMessageSize) & 0x0FU;

            destOffset       = static_cast<FrameSizeType>(offset) + 1U;
            consumedDataSize = static_cast<FrameSizeType>(pendingMessageSize);
            minFrameSize     = _config._singleFrameSize._min;
        }
        else if (fitsIntoLongSingleFrame(pendingMessageSize))
        {
            payload[offset]      = static_cast<uint8_t>(FrameType::SINGLE) << 4U;
            payload[offset + 1U] = static_cast<uint8_t>(pendingMessageSize);
            destOffset           = static_cast<FrameSizeType>(offset) + 2U;
            consumedDataSize     = static_cast<FrameSizeType>(pendingMessageSize);
            minFrameSize         = _config._singleFrameSize._min;
        }
        else if (consecutiveFrameDataSize > 0U)
        {
            destOffset       = static_cast<FrameSizeType>(offset) + 2U;
            consumedDataSize = consecutiveFrameDataSize - 1U;

            if (pendingMessageSize <= ESCAPED_SEQ_MESSAGE_SIZE)
            {
                etl::be_uint16_ext_t{&payload[offset]} = pendingMessageSize & 0xFFFU;
            }
            else
            {
                payload[offset]                             = 0U;
                payload[offset + 1U]                        = 0U;
                etl::be_uint32_ext_t{&payload[offset + 2U]} = pendingMessageSize;

                destOffset += 4U;
                consumedDataSize -= 4U;
            }
            payload[offset] |= static_cast<uint8_t>(FrameType::FIRST) << 4U;
            minFrameSize = _config._firstFrameSize._min;
        }
        else
        {
            return CodecResult::INVALID_FRAME_INDEX;
        }
    }
    else if (consecutiveFrameDataSize > 0U)
    {
        uint8_t const offset = _config._offset;

        payload[offset] = static_cast<uint8_t>(FrameType::CONSECUTIVE) << 4U;
        payload[offset] |= static_cast<uint8_t>(frameIndex) & 0x0FU;

        destOffset       = offset + 1U;
        consumedDataSize = static_cast<FrameSizeType>(
            ::etl::min(static_cast<MessageSizeType>(consecutiveFrameDataSize), pendingMessageSize));
        minFrameSize = ::etl::max(consumedDataSize, _config._consecutiveFrameSize._min);
    }
    else
    {
        return CodecResult::INVALID_FRAME_INDEX;
    }

    FrameSizeType const payloadSize = (destOffset + consumedDataSize);
    if (payloadSize > payload.size())
    {
        return CodecResult::INVALID_FRAME_SIZE;
    }

    (void)::etl::mem_copy(
        &data[0U],
        static_cast<size_t>(consumedDataSize),
        &payload[static_cast<size_t>(destOffset)]);
    return adjustFrame(payload, payloadSize, minFrameSize);
}

template<class DataLinkLayer>
CodecResult DoCanFrameCodec<DataLinkLayer>::encodeFlowControlFrame(
    ::etl::span<uint8_t>& payload,
    FlowStatus const flowStatus,
    uint8_t const blockSize,
    uint8_t const encodedMinSeparationTime) const
{
    size_t const offset = _config._offset;
    if (payload.size() < (offset + 3U))
    {
        return CodecResult::INVALID_FRAME_SIZE;
    }

    payload[offset] = static_cast<uint8_t>(FrameType::FLOW_CONTROL) << 4U;
    payload[offset] |= static_cast<uint8_t>(flowStatus) & 0x0FU;

    payload[offset + 1U] = blockSize;
    payload[offset + 2U] = encodedMinSeparationTime;
    return adjustFrame(
        payload, static_cast<FrameSizeType>(offset) + 3U, _config._flowControlFrameSize._min);
}

template<class DataLinkLayer>
CodecResult DoCanFrameCodec<DataLinkLayer>::adjustFrame(
    ::etl::span<uint8_t>& payload,
    FrameSizeType const payloadSize,
    FrameSizeType const minFrameSize) const
{
    FrameSizeType paddedFrameSize = ::etl::max(payloadSize, minFrameSize);
    if (!_mapper.mapFrameSize(paddedFrameSize))
    {
        return CodecResult::INVALID_FRAME_SIZE;
    }

    if (paddedFrameSize > payloadSize)
    {
        if (paddedFrameSize > payload.size())
        {
            return CodecResult::INVALID_FRAME_SIZE;
        }

        ::etl::mem_set(
            &payload[static_cast<size_t>(payloadSize)],
            static_cast<size_t>(paddedFrameSize) - payloadSize,
            _config._filler);
    }
    payload = payload.first(static_cast<size_t>(paddedFrameSize));
    return CodecResult::OK;
}

template<class DataLinkLayer>
bool DoCanFrameCodec<DataLinkLayer>::checkFrameSize(
    ::etl::span<uint8_t const> const& payload,
    FrameSizeType const minPayloadSize,
    typename DoCanFrameCodecConfig<FrameSizeType>::SizeConfig const frameSize) const
{
    return (payload.size() >= static_cast<size_t>(_config._offset) + minPayloadSize)
           && (payload.size() >= static_cast<size_t>(frameSize._min))
           && (payload.size() <= static_cast<size_t>(frameSize._max));
}
} // namespace docan
