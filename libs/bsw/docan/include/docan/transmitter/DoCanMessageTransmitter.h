// Copyright 2024 Accenture.

#pragma once

#include "docan/common/DoCanConstants.h"
#include "docan/common/DoCanTimerManagement.h"
#include "docan/transmitter/DoCanMessageTransmitProtocolHandler.h"

#include <etl/intrusive_list.h>
#include <etl/span.h>
#include <transport/ITransportMessageProcessedListener.h>
#include <transport/TransportMessage.h>

#include <platform/estdint.h>

namespace docan
{
template<class DataLinkLayer>
class DoCanFrameCodec;

/**
 * Class holding state and data for transmission of a single transport message.
 * \tparam DataLinkLayer class providing data link functionality
 */
template<class DataLinkLayer>
class DoCanMessageTransmitter
: public DoCanMessageTransmitProtocolHandler<typename DataLinkLayer::FrameIndexType>
, public ::etl::bidirectional_link<0>
{
public:
    using DataLinkLayerType       = DataLinkLayer;
    using DataLinkAddressType     = typename DataLinkLayerType::AddressType;
    using DataLinkAddressPairType = typename DataLinkLayerType::AddressPairType;
    using MessageSizeType         = typename DataLinkLayerType::MessageSizeType;
    using FrameIndexType          = typename DataLinkLayerType::FrameIndexType;
    using FrameSizeType           = typename DataLinkLayerType::FrameSizeType;
    using JobHandleType           = typename DataLinkLayerType::JobHandleType;
    using FrameCodecType          = DoCanFrameCodec<DataLinkLayerType>;

    /**
     * Constructor.
     * \param jobHandle handle identifying the send job
     * \param codec codec used to encode outgoing frames
     * \param dataLinkAddressPair reference to pair of reception and transmission address
     *        for incoming and outgoing CAN frames.
     * \param message reference to the transport message holding all transport data
     * \param notificationListener pointer to notificationListener (may be null)
     * \param frameCount total number of frames to send for the message
     * \param consecutiveFrameDataSize maximum data size of consecutive data frames
     */
    DoCanMessageTransmitter(
        JobHandleType jobHandle,
        FrameCodecType const& codec,
        DataLinkAddressPairType const& dataLinkAddressPair,
        ::transport::TransportMessage& message,
        ::transport::ITransportMessageProcessedListener* notificationListener,
        FrameIndexType frameCount,
        FrameSizeType consecutiveFrameDataSize);

    /**
     * Get the handle that identifies the job to the callback.
     * \return unique handle
     */
    JobHandleType getJobHandle() const;

    /**
     * Get the transport message to send.
     * \return pointer to the transport message if sending is in progress
     */
    ::transport::TransportMessage& getMessage() const;

    /**
     * Get the optional notification listener connected with message.
     * \return pointer to listener if defined
     */
    ::transport::ITransportMessageProcessedListener* getNotificationListener() const;

    /**
     * Get the reception address for incoming frames. This address
     * will typically be used to find the message receiver instance for a
     * flow control frame.
     * \return reception address
     */
    DataLinkAddressType getReceptionAddress() const;

    /**
     * Get the transmission address for outgoing frames.
     * \return transmission address
     */
    DataLinkAddressType getTransmissionAddress() const;

    /**
     * Get the data that has not been sent yet.
     * \return slice holding the data
     */
    ::etl::span<uint8_t const> getSendData() const;

    /**
     * Get the maximum data size of consecutive frames.
     * \return maximum consecutive frame data size
     */
    FrameSizeType getConsecutiveFrameDataSize() const;

    /**
     * Get the minimum separation time of the last flow control frame.
     * \return min separation (unit: us)
     */
    uint32_t getMinSeparationTimeUs() const;

    /**
     * Check if the message transmitter is sending consecutive frames
     */
    bool isSendingConsecutiveFrames() const;

    /**
     * Get current frame codec used to encode outgoing data frames.
     */
    FrameCodecType const& getFrameCodec() const;

    /**
     * Called to process a received flow control frame
     * \param flowStatus flow status field
     * \param blockSize received block size
     * \param minSeparationTime the min separation time has been received
     * \param maxWaitCount maximum number of accepted consecutive wait frames
     * \return result indicating state transition
     */
    TransmitResult handleFlowControl(
        FlowStatus flowStatus,
        uint8_t blockSize,
        uint32_t minSeparationTimeUs,
        uint8_t maxWaitCount);

    /**
     * Indicate that one or more frames have been sent.
     * \param frameCount number of frames sent
     * \return result indicating state transition
     */
    TransmitResult framesSent(FrameIndexType frameCount, MessageSizeType dataSize);

    /**
     * Release the transmitter.
     */
    void release();

    /**
     * Send result of the transmission to the notification listener (if given).
     */
    void sendResult();

    /**
     * Internal logic to check to see if the passed in current system microsecond value is
     * greater than the timer expiry time \param nowUs current system microsecond value \return
     * true if the current timer has expired
     */
    bool timerExpired(uint32_t nowUs) const;

    /**
     * Set a timer value.
     * \param nextExpiryUs future microsecond value at which point the timer expires
     * \param isConsecutiveFrames if the timer is for sending the next consecutive frame or not
     */
    void setTimer(uint32_t nextExpiryUs, bool isConsecutiveFrames = false);

    /**
     * Check if the timer has expired
     * \param nowUs the current system microsecond value
     * \return true if the timer has expired, false if not or no timer is currently set
     */
    bool updateTimer(uint32_t nowUs);

    bool operator<(DoCanMessageTransmitter<DataLinkLayer> const& rhs) const
    {
        if (!_isTimerSet)
        {
            return false;
        }
        if (!rhs._isTimerSet)
        {
            return true;
        }
        return timermanagement::less(_timer, rhs._timer);
    }

private:
    FrameCodecType const& _codec;
    ::transport::TransportMessage& _message;
    ::transport::ITransportMessageProcessedListener* const _notificationListener;
    DataLinkAddressType _receptionAddress;
    DataLinkAddressType _transmissionAddress;
    uint32_t _minSeparationTimeUs;
    uint32_t _timer;
    JobHandleType _jobHandle;
    MessageSizeType _bytesSent;
    FrameSizeType const _consecutiveFrameDataSize;
    bool _isTimerSet;
    bool _isSendingConsecutiveFrames;
};

/**
 * Inline implementation.
 */
template<class DataLinkLayer>
DoCanMessageTransmitter<DataLinkLayer>::DoCanMessageTransmitter(
    JobHandleType const jobHandle,
    FrameCodecType const& codec,
    DataLinkAddressPairType const& dataLinkAddressPair,
    ::transport::TransportMessage& message,
    ::transport::ITransportMessageProcessedListener* const notificationListener,
    FrameIndexType const frameCount,
    FrameSizeType const consecutiveFrameDataSize)
: DoCanMessageTransmitProtocolHandler<FrameIndexType>(frameCount)
, ::etl::bidirectional_link<0>()
, _codec(codec)
, _message(message)
, _notificationListener(notificationListener)
, _receptionAddress(dataLinkAddressPair.getReceptionAddress())
, _transmissionAddress(dataLinkAddressPair.getTransmissionAddress())
, _minSeparationTimeUs(0U)
, _timer(0U)
, _jobHandle(jobHandle)
, _bytesSent(0U)
, _consecutiveFrameDataSize(consecutiveFrameDataSize)
, _isTimerSet(false)
, _isSendingConsecutiveFrames(false)
{}

template<class DataLinkLayer>
inline typename DoCanMessageTransmitter<DataLinkLayer>::JobHandleType
DoCanMessageTransmitter<DataLinkLayer>::getJobHandle() const
{
    return _jobHandle;
}

template<class DataLinkLayer>
inline ::transport::TransportMessage& DoCanMessageTransmitter<DataLinkLayer>::getMessage() const
{
    return _message;
}

template<class DataLinkLayer>
inline ::transport::ITransportMessageProcessedListener*
DoCanMessageTransmitter<DataLinkLayer>::getNotificationListener() const
{
    return _notificationListener;
}

template<class DataLinkLayer>
inline typename DoCanMessageTransmitter<DataLinkLayer>::DataLinkAddressType
DoCanMessageTransmitter<DataLinkLayer>::getReceptionAddress() const
{
    return _receptionAddress;
}

template<class DataLinkLayer>
inline typename DoCanMessageTransmitter<DataLinkLayer>::DataLinkAddressType
DoCanMessageTransmitter<DataLinkLayer>::getTransmissionAddress() const
{
    return _transmissionAddress;
}

template<class DataLinkLayer>
inline ::etl::span<uint8_t const> DoCanMessageTransmitter<DataLinkLayer>::getSendData() const
{
    return ::etl::span<uint8_t const>(
        &_message.getPayload()[_bytesSent],
        static_cast<size_t>(_message.getPayloadLength()) - static_cast<size_t>(_bytesSent));
}

template<class DataLinkLayer>
inline typename DoCanMessageTransmitter<DataLinkLayer>::FrameSizeType
DoCanMessageTransmitter<DataLinkLayer>::getConsecutiveFrameDataSize() const
{
    return _consecutiveFrameDataSize;
}

template<class DataLinkLayer>
uint32_t DoCanMessageTransmitter<DataLinkLayer>::getMinSeparationTimeUs() const
{
    return _minSeparationTimeUs;
}

template<class DataLinkLayer>
bool DoCanMessageTransmitter<DataLinkLayer>::isSendingConsecutiveFrames() const
{
    return _isSendingConsecutiveFrames;
}

template<class DataLinkLayer>
typename DoCanMessageTransmitter<DataLinkLayer>::FrameCodecType const&
DoCanMessageTransmitter<DataLinkLayer>::getFrameCodec() const
{
    return _codec;
}

template<class DataLinkLayer>
TransmitResult DoCanMessageTransmitter<DataLinkLayer>::handleFlowControl(
    FlowStatus const flowStatus,
    uint8_t const blockSize,
    uint32_t const minSeparationTimeUs,
    uint8_t const maxWaitCount)
{
    TransmitResult const result
        = DoCanMessageTransmitProtocolHandler<FrameIndexType>::handleFlowControl(
            flowStatus, blockSize, minSeparationTimeUs > 0U, maxWaitCount);
    if (result.getActionSet().test(TransmitAction::STORE_SEPARATION_TIME))
    {
        _minSeparationTimeUs = minSeparationTimeUs;
    }
    return result;
}

template<class DataLinkLayer>
TransmitResult DoCanMessageTransmitter<DataLinkLayer>::framesSent(
    FrameIndexType const frameCount, MessageSizeType const dataSize)
{
    _bytesSent += dataSize;
    return DoCanMessageTransmitProtocolHandler<FrameIndexType>::framesSent(frameCount);
}

template<class DataLinkLayer>
void DoCanMessageTransmitter<DataLinkLayer>::release()
{
    _jobHandle           = JobHandleType();
    _receptionAddress    = DataLinkLayer::INVALID_ADDRESS;
    _transmissionAddress = DataLinkLayer::INVALID_ADDRESS;
}

template<class DataLinkLayer>
bool DoCanMessageTransmitter<DataLinkLayer>::timerExpired(uint32_t const nowUs) const
{
    // Less than or equal
    return timermanagement::less(_timer, nowUs + 1);
}

template<class DataLinkLayer>
void DoCanMessageTransmitter<DataLinkLayer>::setTimer(
    uint32_t const nextExpiryUs, bool const isConsecutiveFrames)
{
    _timer                      = nextExpiryUs;
    _isTimerSet                 = true;
    _isSendingConsecutiveFrames = isConsecutiveFrames;
}

template<class DataLinkLayer>
bool DoCanMessageTransmitter<DataLinkLayer>::updateTimer(uint32_t const nowUs)
{
    if (!_isTimerSet)
    {
        return false;
    }

    if (timerExpired(nowUs))
    {
        _isTimerSet = false;
        return true;
    }
    return false;
}

} // namespace docan
