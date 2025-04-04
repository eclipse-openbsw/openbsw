// Copyright 2024 Accenture.

#pragma once

#include "can/canframes/CANFrame.h"
#include "can/canframes/ICANFrameSentListener.h"
#include "can/framemgmt/ICANFrameListener.h"
#include "can/transceiver/ICanTransceiver.h"
#include "docan/addressing/IDoCanAddressConverter.h"
#include "docan/common/DoCanConnection.h"
#include "docan/common/DoCanConstants.h"
#include "docan/datalink/DoCanFrameCodec.h"
#include "docan/datalink/DoCanFrameDecoder.h"
#include "docan/datalink/IDoCanFlowControlFrameTransmitter.h"
#include "docan/datalink/IDoCanFrameReceiver.h"
#include "docan/datalink/IDoCanPhysicalTransceiver.h"

namespace docan
{
/**
 * DoCAN implementation of a physical CAN transceiver.
 * \tparam Addressing class providing addressing used for encoding/decoding CAN frames
 */
template<class Addressing>
class DoCanPhysicalCanTransceiver final
: public IDoCanPhysicalTransceiver<typename Addressing::DataLinkLayerType>
, private ::can::ICANFrameListener
, public ::can::ICANFrameSentListener
{
public:
    using AddressingType       = Addressing;
    using DataLinkLayerType    = typename AddressingType::DataLinkLayerType;
    using FrameTransmitterType = IDoCanDataFrameTransmitter<DataLinkLayerType>;
    using FrameCodecType       = DoCanFrameCodec<DataLinkLayerType>;
    using DataLinkAddressType  = typename DataLinkLayerType::AddressType;
    using MessageSizeType      = typename DataLinkLayerType::MessageSizeType;
    using FrameIndexType       = typename DataLinkLayerType::FrameIndexType;
    using FrameSizeType        = typename DataLinkLayerType::FrameSizeType;
    using JobHandleType        = typename FrameTransmitterType::JobHandleType;

    /**
     * Constructor.
     * \param transceiver CAN transceiver
     * \param filter filter to provider for CAN listener
     * \param codec Codec class
     */
    DoCanPhysicalCanTransceiver(
        ::can::ICanTransceiver& transceiver,
        ::can::IFilter& filter,
        IDoCanAddressConverter<DataLinkLayerType> const& addressConverter,
        AddressingType const& addressing);

    void init(IDoCanFrameReceiver<DataLinkLayerType>& receiver) override;
    void shutdown() override;

    SendResult startSendDataFrames(
        FrameCodecType const& codec,
        IDoCanDataFrameTransmitterCallback<DataLinkLayerType>& callback,
        JobHandleType jobHandle,
        DataLinkAddressType transmissionAddress,
        FrameIndexType firstFrameIndex,
        FrameIndexType lastFrameIndex,
        FrameSizeType consecutiveFrameDataSize,
        ::estd::slice<uint8_t const> const& data) override;
    void cancelSendDataFrames(
        IDoCanDataFrameTransmitterCallback<DataLinkLayerType>& callback,
        JobHandleType jobHandle) override;

    bool sendFlowControl(
        FrameCodecType const& codec,
        DataLinkAddressType transmissionAddress,
        FlowStatus flowStatus,
        uint8_t blockSize,
        uint8_t encodedMinSeparationTime) override;

    ::can::ICANFrameListener& getListener();

private:
    void frameReceived(::can::CANFrame const& canFrame) override;
    ::can::IFilter& getFilter() override;
    void canFrameSent(::can::CANFrame const& frame) override;

    ::can::CANFrame _frame;
    ::can::ICanTransceiver& _transceiver;
    ::can::IFilter& _filter;
    IDoCanAddressConverter<DataLinkLayerType> const& _addressConverter;
    AddressingType const& _addressing;
    IDoCanFrameReceiver<DataLinkLayerType>* _frameReceiver;
    IDoCanDataFrameTransmitterCallback<DataLinkLayerType>* _sendCallback;
    JobHandleType _sendJobHandle;
    uint8_t _sendDataSize;
    bool _sendPending;
};

/**
 * Inline implementation.
 */
template<class Addressing>
DoCanPhysicalCanTransceiver<Addressing>::DoCanPhysicalCanTransceiver(
    ::can::ICanTransceiver& transceiver,
    ::can::IFilter& filter,
    IDoCanAddressConverter<DataLinkLayerType> const& addressConverter,
    AddressingType const& addressing)
: _frame()
, _transceiver(transceiver)
, _filter(filter)
, _addressConverter(addressConverter)
, _addressing(addressing)
, _frameReceiver(nullptr)
, _sendCallback(nullptr)
, _sendJobHandle()
, _sendDataSize(0U)
, _sendPending(false)
{}

template<class Addressing>
void DoCanPhysicalCanTransceiver<Addressing>::init(IDoCanFrameReceiver<DataLinkLayerType>& receiver)
{
    _frameReceiver = &receiver;
    _transceiver.addCANFrameListener(*this);
}

template<class Addressing>
void DoCanPhysicalCanTransceiver<Addressing>::shutdown()
{
    _frameReceiver = nullptr;
    _transceiver.removeCANFrameListener(*this);
}

template<class Addressing>
SendResult DoCanPhysicalCanTransceiver<Addressing>::startSendDataFrames(
    FrameCodecType const& codec,
    IDoCanDataFrameTransmitterCallback<DataLinkLayerType>& callback,
    JobHandleType const jobHandle,
    DataLinkAddressType const transmissionAddress,
    FrameIndexType const firstFrameIndex,
    FrameIndexType const lastFrameIndex,
    FrameSizeType const consecutiveFrameDataSize,
    ::estd::slice<uint8_t const> const& data)
{
    (void)lastFrameIndex;
    if (!_sendPending)
    {
        ::estd::slice<uint8_t> payload = ::estd::slice<uint8_t>::from_pointer(
            _frame.getPayload(), static_cast<size_t>(_frame.getMaxPayloadLength()));

        if (codec.encodeDataFrame(
                payload, data, firstFrameIndex, consecutiveFrameDataSize, _sendDataSize)
            != CodecResult::OK)
        {
            return SendResult::INVALID;
        }

        uint32_t canId;
        _addressing.encodeTransmissionAddress(transmissionAddress, canId, payload);
        _frame.setId(canId);
        _frame.setPayloadLength(static_cast<uint8_t>(payload.size()));
        ::can::ICanTransceiver::ErrorCode const result = _transceiver.write(_frame, *this);
        if (result == ::can::ICanTransceiver::ErrorCode::CAN_ERR_OK)
        {
            _sendPending   = true;
            _sendCallback  = &callback;
            _sendJobHandle = jobHandle;
            return SendResult::QUEUED_FULL;
        }
        if (result != ::can::ICanTransceiver::ErrorCode::CAN_ERR_TX_HW_QUEUE_FULL)
        {
            return SendResult::FAILED;
        }
        // default return value: FULL
    }
    return SendResult::FULL;
}

template<class Addressing>
void DoCanPhysicalCanTransceiver<Addressing>::cancelSendDataFrames(
    IDoCanDataFrameTransmitterCallback<DataLinkLayerType>& callback, JobHandleType const jobHandle)
{
    // This is done under lock in DoCanTransmitter.
    if (_sendPending && (_sendCallback == &callback) && (_sendJobHandle == jobHandle))
    {
        _sendPending  = false;
        _sendCallback = nullptr;
    }
}

template<class Addressing>
bool DoCanPhysicalCanTransceiver<Addressing>::sendFlowControl(
    FrameCodecType const& codec,
    DataLinkAddressType const transmissionAddress,
    FlowStatus const flowStatus,
    uint8_t const blockSize,
    uint8_t const encodedMinSeparationTime)
{
    ::can::CANFrame frame;

    ::estd::slice<uint8_t> payload = ::estd::slice<uint8_t>::from_pointer(
        frame.getPayload(), static_cast<size_t>(frame.getMaxPayloadLength()));
    (void)codec.encodeFlowControlFrame(payload, flowStatus, blockSize, encodedMinSeparationTime);
    uint32_t canId;
    _addressing.encodeTransmissionAddress(transmissionAddress, canId, payload);
    frame.setId(canId);
    frame.setPayloadLength(static_cast<uint8_t>(payload.size()));
    return _transceiver.write(frame) == ::can::ICanTransceiver::ErrorCode::CAN_ERR_OK;
}

template<class Addressing>
void DoCanPhysicalCanTransceiver<Addressing>::frameReceived(::can::CANFrame const& canFrame)
{
    if (_frameReceiver == nullptr)
    {
        return;
    }

    ::estd::slice<uint8_t const> const payload = ::estd::slice<uint8_t const>::from_pointer(
        canFrame.getPayload(), canFrame.getPayloadLength());
    DataLinkAddressType const receptionAddress
        = _addressing.decodeReceptionAddress(canFrame.getId(), payload);

    DoCanTransportAddressPair transportAddressPair;
    DataLinkAddressType transmissionAddress;
    FrameCodecType const* const codec = _addressConverter.getReceptionParameters(
        receptionAddress, transportAddressPair, transmissionAddress);
    if (codec == nullptr)
    {
        return;
    }

    DoCanConnection<DataLinkLayerType> const connection(
        *codec,
        DoCanDataLinkAddressPair<DataLinkAddressType>(receptionAddress, transmissionAddress),
        transportAddressPair);
    (void)DoCanFrameDecoder<DataLinkLayerType>::decodeFrame(connection, payload, *_frameReceiver);
}

template<class Addressing>
::can::IFilter& DoCanPhysicalCanTransceiver<Addressing>::getFilter()
{
    return _filter;
}

template<class Addressing>
void DoCanPhysicalCanTransceiver<Addressing>::canFrameSent(::can::CANFrame const& frame)
{
    (void)frame;
    if (_sendPending)
    {
        _sendPending = false;
        if (_sendCallback != nullptr)
        {
            _sendCallback->dataFramesSent(_sendJobHandle, 1U, _sendDataSize);
        }
    }
}

template<class Addressing>
::can::ICANFrameListener& DoCanPhysicalCanTransceiver<Addressing>::getListener()
{
    return *this;
}
} // namespace docan
