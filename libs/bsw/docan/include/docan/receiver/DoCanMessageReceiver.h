// Copyright 2024 Accenture.

#pragma once

#include "docan/common/DoCanConnection.h"
#include "docan/common/DoCanTimerManagement.h"
#include "docan/common/DoCanTransportAddressPair.h"
#include "docan/receiver/DoCanMessageReceiveProtocolHandler.h"

#include <transport/ITransportMessageProcessedListener.h>
#include <transport/TransportMessage.h>

#include <estd/forward_list.h>
#include <platform/estdint.h>

#include <limits>

namespace docan
{
/**
 * Class holding state and data for reception of a single transport message.
 * \tparam DataLinkLayer class providing data link functionality
 */
template<class DataLinkLayer>
class DoCanMessageReceiver
: public DoCanMessageReceiveProtocolHandler<typename DataLinkLayer::FrameIndexType>
, public ::estd::forward_list_node<DoCanMessageReceiver<DataLinkLayer>>
{
public:
    using DataLinkLayerType       = DataLinkLayer;
    using DataLinkAddressType     = typename DataLinkLayerType::AddressType;
    using DataLinkAddressPairType = typename DataLinkLayerType::AddressPairType;
    using MessageSizeType         = typename DataLinkLayerType::MessageSizeType;
    using FrameIndexType          = typename DataLinkLayerType::FrameIndexType;
    using FrameSizeType           = typename DataLinkLayerType::FrameSizeType;
    using ConnectionType          = DoCanConnection<DataLinkLayerType>;

    /**
     * Constructor.
     * \param connection holds connection-related data, such as transport & data link addresses
     * \param messageSize size of message
     * \param frameCount number of expected frames for this transfer
     * \param consecutiveFrameDataSize expected size of a regular consecutive frame
     * \param maxBlockSize maximum block size for this transfer
     * \param encodedMinSeparationTime encoded min separation time for this transfer
     * \param firstFrameData reference to frame data
     * \param blocked flag that indicates whether the receiver should be marked as blocked
     */
    DoCanMessageReceiver(
        ConnectionType const& connection,
        MessageSizeType messageSize,
        FrameIndexType frameCount,
        FrameSizeType consecutiveFrameDataSize,
        uint8_t maxBlockSize,
        uint8_t encodedMinSeparationTime,
        ::estd::slice<uint8_t const> const& firstFrameData,
        bool blocked);

    /**
     * Get the allocated transport message if existing.
     * \return pointer to message if existing
     */
    ::transport::TransportMessage* getMessage() const;

    /**
     * Get the transport address pair representing the current connection.
     * \return reference to transport address pair
     */
    DoCanTransportAddressPair const& getTransportAddressPair() const;

    /**
     * Get the reception address of incoming frames. This address will typically
     * be used to find the message receiver instance for a consecutive CAN frame.
     * \return reception address
     */
    DataLinkAddressType getReceptionAddress() const;

    /**
     * Get the transmission address for outgoing flow control frames.
     * \return transmission address
     */
    DataLinkAddressType getTransmissionAddress() const;

    /**
     * Get the connection codec for outgoing flow control frames.
     * \return transmission address
     */
    DoCanFrameCodec<DataLinkLayerType> const& getFrameCodec() const;

    /**
     * Get the (expected) size of the received message.
     * \return message size in number of bytes
     */
    MessageSizeType getMessageSize() const;

    /**
     * Get the maximum block size for this transfer.
     * \return max block size for this transfer
     */
    uint8_t getMaxBlockSize() const;

    /**
     * Get the encoded minimum separation time.
     * \return encoded minimum separation time
     */
    uint8_t getEncodedMinSeparationTime() const;

    /**
     * Called to announce the result of a message allocation try.
     * \param message pointer to message, 0 indicates no success
     * \param maxRetryCount maximum number of allowed retries
     * \return result indicating state transition
     */
    ReceiveResult allocated(::transport::TransportMessage* message, uint8_t maxRetryCount);

    /**
     * Return whether a consecutive frame is expected
     * \return true if a consecutive frame is expected
     */
    bool isConsecutiveFrameExpected() const;

    /**
     * Get expected length of data for next consecutive frame.
     * \return expected data length if expected, 0 if no consecutive frame is expected
     */
    FrameSizeType getExpectedConsecutiveFrameDataSize() const;

    /**
     * Called to announce the reception of a consecutive frame
     * \param sequenceNumber sequence number of received consecutive frame
     * \param expectedSize expected size as returned from getExpectedConsecutiveFrameDataSize()
     * \param data reference to the data of the received frame
     * \return result indicating state transition
     */
    ReceiveResult consecutiveFrameReceived(
        uint8_t sequenceNumber,
        FrameSizeType expectedSize,
        ::estd::slice<uint8_t const> const& data);

    /**
     * Detach a transport message. Reception address and message are kept.
     * \return detached TransportMessage, 0L if none was attached
     */
    ::transport::TransportMessage* detachMessage();

    /**
     * Release the message. Reset reception address and message.
     * \return TransportMessage connected with this receivers, 0L if not set allocated
     */
    ::transport::TransportMessage* release();

    /**
     * Internal logic to check to see if the passed in current system microsecond value is greater
     * than the timer expiry time
     * \param nowUs current system microsecond value
     * \return true if the current timer has expired
     */
    bool timerExpired(uint32_t nowUs) const;

    /**
     * Set a timer value.
     * \param nextExpiryUs future microsecond value at which point the timer expires
     */
    void setTimer(uint32_t nextExpiryUs);

    /**
     * Check if the timer has expired
     * \param nowUs the current system microsecond value
     * \return true if the timer has expired, false if not or no timer is currently set
     */
    bool updateTimer(uint32_t nowUs);

    /**
     * Check whether the receiver is marked as blocked.
     * \return true if blocked
     */
    bool isBlocked() const;

    /**
     * Set the new value of the blocked flag
     * \param blocked true if the receiver is marked as blocked
     */
    void setBlocked(bool blocked);

    /**
     * Get the first frame data
     * \return first frame data
     */
    ::estd::slice<uint8_t const> getFirstFrameData() const;

    bool operator<(DoCanMessageReceiver<DataLinkLayer> const& rhs) const
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
    ConnectionType const _connection;
    ::transport::TransportMessage* _message;
    uint8_t const* const _firstFrameData;
    uint32_t _timer;
    MessageSizeType const _messageSize;
    FrameSizeType const _firstFrameDataSize;
    FrameSizeType const _consecutiveFrameDataSize;
    bool _isTimerSet;
    uint8_t const _maxBlockSize;
    uint8_t const _encodedMinSeparationTime;
    bool _blocked;
};

namespace declare
{
/**
 * Helper class that can be used to determine the size to be allocated for a
 * message receiver together with first frame data.
 */
template<class DataLinkLayer, typename DataLinkLayer::FrameSizeType FIRST_FRAME_DATA_SIZE>
class DoCanMessageReceiver
{
private:
    ::docan::DoCanMessageReceiver<DataLinkLayer> _receiver;
    uint8_t _firstFrameData[FIRST_FRAME_DATA_SIZE];
};

} // namespace declare

/**
 * Inline implementation.
 */
template<class DataLinkLayer>
DoCanMessageReceiver<DataLinkLayer>::DoCanMessageReceiver(
    ConnectionType const& connection,
    MessageSizeType const messageSize,
    FrameIndexType const frameCount,
    FrameSizeType const consecutiveFrameDataSize,
    uint8_t const maxBlockSize,
    uint8_t const encodedMinSeparationTime,
    ::estd::slice<uint8_t const> const& firstFrameData,
    bool const blocked)
: DoCanMessageReceiveProtocolHandler<FrameIndexType>(frameCount)
, ::estd::forward_list_node<DoCanMessageReceiver<DataLinkLayer>>()
, _connection(connection)
, _message(nullptr)
, _firstFrameData(firstFrameData.data())
, _timer(0U)
, _messageSize(messageSize)
, _firstFrameDataSize(static_cast<FrameSizeType>(firstFrameData.size()))
, _consecutiveFrameDataSize(consecutiveFrameDataSize)
, _isTimerSet(false)
, _maxBlockSize(maxBlockSize)
, _encodedMinSeparationTime(encodedMinSeparationTime)
, _blocked(blocked)
{}

template<class DataLinkLayer>
inline ::transport::TransportMessage* DoCanMessageReceiver<DataLinkLayer>::getMessage() const
{
    return _message;
}

template<class DataLinkLayer>
inline DoCanTransportAddressPair const&
DoCanMessageReceiver<DataLinkLayer>::getTransportAddressPair() const
{
    return _connection.getTransportAddressPair();
}

template<class DataLinkLayer>
inline typename DoCanMessageReceiver<DataLinkLayer>::DataLinkAddressType
DoCanMessageReceiver<DataLinkLayer>::getReceptionAddress() const
{
    return _connection.getDataLinkAddressPair().getReceptionAddress();
}

template<class DataLinkLayer>
inline typename DoCanMessageReceiver<DataLinkLayer>::DataLinkAddressType
DoCanMessageReceiver<DataLinkLayer>::getTransmissionAddress() const
{
    return _connection.getDataLinkAddressPair().getTransmissionAddress();
}

template<class DataLinkLayer>
inline DoCanFrameCodec<DataLinkLayer> const&
DoCanMessageReceiver<DataLinkLayer>::getFrameCodec() const
{
    return _connection.getFrameCodec();
}

template<class DataLinkLayer>
inline typename DoCanMessageReceiver<DataLinkLayer>::MessageSizeType
DoCanMessageReceiver<DataLinkLayer>::getMessageSize() const
{
    return _messageSize;
}

template<class DataLinkLayer>
inline uint8_t DoCanMessageReceiver<DataLinkLayer>::getMaxBlockSize() const
{
    return _maxBlockSize;
}

template<class DataLinkLayer>
inline uint8_t DoCanMessageReceiver<DataLinkLayer>::getEncodedMinSeparationTime() const
{
    return _encodedMinSeparationTime;
}

template<class DataLinkLayer>
ReceiveResult DoCanMessageReceiver<DataLinkLayer>::allocated(
    ::transport::TransportMessage* const message, uint8_t const maxRetryCount)
{
    auto const result = DoCanMessageReceiveProtocolHandler<FrameIndexType>::allocated(
        message != nullptr, maxRetryCount);
    if ((!DoCanMessageReceiveProtocolHandler<FrameIndexType>::isAllocating())
        && (message != nullptr))
    {
        _message = message;
        (void)_message->append(_firstFrameData, static_cast<FrameSizeType>(_firstFrameDataSize));
    }
    return result;
}

template<class DataLinkLayer>
inline bool DoCanMessageReceiver<DataLinkLayer>::isConsecutiveFrameExpected() const
{
    return _message != nullptr;
}

template<class DataLinkLayer>
inline typename DoCanMessageReceiver<DataLinkLayer>::FrameSizeType
DoCanMessageReceiver<DataLinkLayer>::getExpectedConsecutiveFrameDataSize() const
{
    return ((_message->validBytes() + _consecutiveFrameDataSize) <= _messageSize)
               ? _consecutiveFrameDataSize
               : static_cast<FrameSizeType>(_messageSize - _message->validBytes());
}

template<class DataLinkLayer>
inline ReceiveResult DoCanMessageReceiver<DataLinkLayer>::consecutiveFrameReceived(
    uint8_t const sequenceNumber,
    FrameSizeType const expectedSize,
    ::estd::slice<uint8_t const> const& data)
{
    auto const result
        = DoCanMessageReceiveProtocolHandler<FrameIndexType>::consecutiveFrameReceived(
            sequenceNumber, _maxBlockSize);
    if (DoCanMessageReceiveProtocolHandler<FrameIndexType>::getState() != ReceiveState::DONE)
    {
        (void)_message->append(data.data(), static_cast<uint16_t>(expectedSize));
    }
    return result;
}

template<class DataLinkLayer>
::transport::TransportMessage* DoCanMessageReceiver<DataLinkLayer>::detachMessage()
{
    ::transport::TransportMessage* const message = _message;
    _message                                     = nullptr;
    return message;
}

template<class DataLinkLayer>
::transport::TransportMessage* DoCanMessageReceiver<DataLinkLayer>::release()
{
    ::transport::TransportMessage* const message = detachMessage();
    return message;
}

template<class DataLinkLayer>
bool DoCanMessageReceiver<DataLinkLayer>::timerExpired(uint32_t const nowUs) const
{
    // Less than or equal
    return timermanagement::less(_timer, nowUs + 1);
}

template<class DataLinkLayer>
inline void DoCanMessageReceiver<DataLinkLayer>::setTimer(uint32_t const nextExpiryUs)
{
    _timer      = nextExpiryUs;
    _isTimerSet = true;
}

template<class DataLinkLayer>
bool DoCanMessageReceiver<DataLinkLayer>::updateTimer(uint32_t const nowUs)
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

template<class DataLinkLayer>
inline bool DoCanMessageReceiver<DataLinkLayer>::isBlocked() const
{
    return _blocked;
}

template<class DataLinkLayer>
inline void DoCanMessageReceiver<DataLinkLayer>::setBlocked(bool const blocked)
{
    _blocked = blocked;
}

template<class DataLinkLayer>
inline ::estd::slice<uint8_t const> DoCanMessageReceiver<DataLinkLayer>::getFirstFrameData() const
{
    return ::estd::slice<uint8_t const>::from_pointer(
        _firstFrameData, static_cast<size_t>(_firstFrameDataSize));
}

} // namespace docan
