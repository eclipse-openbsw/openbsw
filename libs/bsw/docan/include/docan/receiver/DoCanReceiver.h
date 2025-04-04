// Copyright 2024 Accenture.

#pragma once

#include "docan/addressing/IDoCanAddressConverter.h"
#include "docan/common/DoCanConnection.h"
#include "docan/common/DoCanConstants.h"
#include "docan/common/DoCanParameters.h"
#include "docan/datalink/IDoCanFlowControlFrameTransmitter.h"
#include "docan/receiver/DoCanMessageReceiver.h"

#include <async/Async.h>
#include <async/Types.h>
#include <async/util/MemberCall.h>
#include <common/busid/BusId.h>
#include <interrupts/SuspendResumeAllInterruptsScopedLock.h>
#include <transport/ITransportMessageProvidingListener.h>
#include <util/estd/derived_object_pool.h>
#include <util/logger/Logger.h>

#include <estd/forward_list.h>
#include <estd/functional.h>
#include <estd/memory.h>
#include <estd/slice.h>

#include <limits>

namespace docan
{
/**
 * Class responsible for parallel reception of transport messages.
 * \tparam DataLinkLayer class providing data link functionality
 */
template<class DataLinkLayer>
class DoCanReceiver : private ::transport::ITransportMessageProcessedListener
{
public:
    using DataLinkLayerType               = DataLinkLayer;
    using DataLinkAddressType             = typename DataLinkLayerType::AddressType;
    using DataLinkAddressPairType         = typename DataLinkLayerType::AddressPairType;
    using MessageSizeType                 = typename DataLinkLayerType::MessageSizeType;
    using FrameIndexType                  = typename DataLinkLayerType::FrameIndexType;
    using FrameSizeType                   = typename DataLinkLayerType::FrameSizeType;
    using ConnectionType                  = DoCanConnection<DataLinkLayerType>;
    using FlowControlFrameTransmitterType = IDoCanFlowControlFrameTransmitter<DataLinkLayerType>;
    using MessageReceiverType             = DoCanMessageReceiver<DataLinkLayerType>;
    using MessageReceiverListType         = ::estd::forward_list<MessageReceiverType>;

    /** Constructor.
     *
     * \param busId bus id of receiver.
     *
     * \param messageProvidingListener reference to interface for transport message handling
     *
     * \param flowControlFrameTransmitter reference to interface for emitting flow control frames
     *
     * \param messageReceiverBlockPool block pool to use for message receivers and first frame data
     *
     * \param parameters reference to object holding parameters. No copy is held, so changes to the
     * parameters will affect further message reception
     */
    DoCanReceiver(
        uint8_t busId,
        ::async::ContextType context,
        ::transport::ITransportMessageProvidingListener& messageProvidingListener,
        FlowControlFrameTransmitterType& flowControlFrameTransmitter,
        ::util::estd::block_pool& messageReceiverBlockPool,
        IDoCanAddressConverter<DataLinkLayerType>& addressConverter,
        DoCanParameters const& parameters,
        uint8_t loggerComponent);

    /**
     * Initializes the receiver.
     */
    void init();

    /**
     * Shuts down the receiver.
     */
    void shutdown();

    /**
     * called on reception of a first data frame (either single or first segmented).
     * \param connection holds connection-related data, such as transport & data link addresses
     * \param messageSize expected message size
     * \param frameCount number of total frames to receive (1 => single frame)
     * \param consecutiveFrameDataSize data size of a regular consecutive frame
     * \param data reference to the data of the frame
     */
    void firstDataFrameReceived(
        ConnectionType const& connection,
        MessageSizeType messageSize,
        FrameIndexType frameCount,
        FrameSizeType consecutiveFrameDataSize,
        ::estd::slice<uint8_t const> const& data);

    /**
     * called on reception of a consecutive data frame.
     * \param receptionAddress reception address of the frame
     * \param sequenceNumber sequence number as received from transport layer
     * \param data reference to frame containing the data
     */
    void consecutiveDataFrameReceived(
        DataLinkAddressType receptionAddress,
        uint8_t sequenceNumber,
        ::estd::slice<uint8_t const> const& data);

    /**
     * Check if any message has been received, or if a timeout has occurred
     */
    void cyclicTask(uint32_t nowUs);

private:
    static uint8_t const FORMAT_BUFFER_SIZE = 32U;

    void processMessageReceivers();

    class RemoveGuard
    {
    public:
        explicit RemoveGuard(DoCanReceiver* receiver);
        ~RemoveGuard();

    private:
        DoCanReceiver* _receiver;
    };

    void transportMessageProcessed(
        ::transport::TransportMessage& transportMessage, ProcessingResult result) override;

    void handleTransitions(
        MessageReceiverType& messageReceiver, ReceiveResult result, char const* functionName);
    void handleResult(
        MessageReceiverType& messageReceiver, ReceiveResult result, char const* functionName);
    ReceiveResult handleTransition(MessageReceiverType& messageReceiver);
    ReceiveResult allocateTransportMessage(MessageReceiverType& messageReceiver, bool peek);
    ReceiveResult sendFlowControlFrame(MessageReceiverType& messageReceiver);
    ReceiveResult startProcessingTransportMessage(MessageReceiverType& messageReceiver);
    ReceiveResult release(MessageReceiverType& messageReceiver);

    bool handlePendingMessageReceivers(DataLinkAddressType receptionAddress);

    void resetTimer(MessageReceiverType& messageReceiver) const;

    MessageReceiverType* findMessageReceiver(DataLinkAddressType receptionAddress);

    void setRemoveLock();
    void releaseRemoveLock();

    char const* getName() const;

    static ::estd::slice<uint8_t const> copyFirstFrameData(
        uint8_t* receiverAddress, ::estd::slice<uint8_t const> const& firstFrameData);

    IDoCanAddressConverter<DataLinkLayerType>& _addressConverter;
    ::transport::ITransportMessageProvidingListener& _messageProvidingListener;
    FlowControlFrameTransmitterType& _flowControlFrameTransmitter;
    ::util::estd::derived_object_pool<MessageReceiverType> _messageReceiverPool;
    ::async::MemberCall<DoCanReceiver, &DoCanReceiver::processMessageReceivers>
        _processMessageReceivers;
    MessageReceiverListType _messageReceivers;
    DoCanParameters const& _parameters;
    FrameSizeType const _maxFirstFrameDataSize;
    ::async::ContextType const _context;
    uint8_t const _busId = 0xFF;
    uint8_t const _loggerComponent;
    uint8_t _removeLockCount;
    uint8_t _releasedReceiverCount;
    bool _timersUpdated;
};

/**
 * Inline implementation.
 */
template<class DataLinkLayer>
DoCanReceiver<DataLinkLayer>::DoCanReceiver(
    uint8_t const busId,
    ::async::ContextType const context,
    ::transport::ITransportMessageProvidingListener& messageProvidingListener,
    FlowControlFrameTransmitterType& flowControlFrameTransmitter,
    ::util::estd::block_pool& messageReceiverBlockPool,
    IDoCanAddressConverter<DataLinkLayerType>& addressConverter,
    DoCanParameters const& parameters,
    uint8_t const loggerComponent)
: _addressConverter(addressConverter)
, _messageProvidingListener(messageProvidingListener)
, _flowControlFrameTransmitter(flowControlFrameTransmitter)
, _messageReceiverPool(messageReceiverBlockPool)
, _processMessageReceivers(*this)
, _messageReceivers()
, _parameters(parameters)
, _maxFirstFrameDataSize(static_cast<FrameSizeType>(
      static_cast<size_t>(messageReceiverBlockPool.block_size()) - sizeof(MessageReceiverType)))
, _context(context)
, _busId(busId)
, _loggerComponent(loggerComponent)
, _removeLockCount(0U)
, _releasedReceiverCount(0U)
, _timersUpdated(false)
{}

template<class DataLinkLayer>
void DoCanReceiver<DataLinkLayer>::init()
{
    // Perform assert checks in init() so we can't possibly assert during construction, which could
    // cause serious problems for some of our projects (HW getting locked due to cyclic resets)
    // Ensure the block pool block size >= MessageReceiverType size, otherwise
    // _maxFirstFrameDataSize will be an invalid garbage value
    estd_assert(_messageReceiverPool.get_block_pool().block_size() >= sizeof(MessageReceiverType));
    // Ensure the following subtraction is less than FrameSizeType's max value, otherwise
    // _maxFirstFrameDataSize will be an invalid garbage value
    estd_assert(
        _messageReceiverPool.get_block_pool().block_size() - sizeof(MessageReceiverType)
        < std::numeric_limits<FrameSizeType>::max());
}

template<class DataLinkLayer>
void DoCanReceiver<DataLinkLayer>::shutdown()
{
    RemoveGuard const guard(this);
    for (typename MessageReceiverListType::iterator it = _messageReceivers.begin();
         it != _messageReceivers.end();
         ++it)
    {
        ::interrupts::SuspendResumeAllInterruptsScopedLock const lock;
        handleTransitions(*it, it->shutdown(), "shutdown");
    }
}

template<class DataLinkLayer>
void DoCanReceiver<DataLinkLayer>::firstDataFrameReceived(
    ConnectionType const& connection,
    MessageSizeType const messageSize,
    FrameIndexType const frameCount,
    FrameSizeType const consecutiveFrameDataSize,
    ::estd::slice<uint8_t const> const& data)
{
    DataLinkAddressPairType const& dataLinkAddressPair = connection.getDataLinkAddressPair();
    // functional routing entries are defined with dest TA as INVALID
    // only functional single frames (frameCount == 1) are allowed
    if ((frameCount > 1U)
        && (dataLinkAddressPair.getTransmissionAddress() == DataLinkLayerType::INVALID_ADDRESS))
    {
        char formatBuffer[FORMAT_BUFFER_SIZE];
        ::util::logger::Logger::warn(
            _loggerComponent,
            "DoCanReceiver(%s)::firstDataFrameReceived(%s): functional first frames (RA: "
            "0x%x) are not allowed!",
            getName(),
            _addressConverter.formatDataLinkAddress(
                connection.getDataLinkAddressPair().getReceptionAddress(), formatBuffer),
            dataLinkAddressPair.getReceptionAddress());
    }
    else if (!_messageReceiverPool.empty())
    {
        RemoveGuard const guard(this);
        // Check data size to avoid memory corruption
        if (data.size() <= _maxFirstFrameDataSize)
        {
            MessageReceiverType* messageReceiver;
            {
                ::interrupts::SuspendResumeAllInterruptsScopedLock const lock;
                ::estd::constructor<MessageReceiverType> constructor
                    = _messageReceiverPool.template allocate<MessageReceiverType>();

                auto const firstFrameCopy = copyFirstFrameData(constructor, data);
                bool const blocked
                    = handlePendingMessageReceivers(dataLinkAddressPair.getReceptionAddress());
                messageReceiver = &constructor.construct(
                    connection,
                    messageSize,
                    frameCount,
                    consecutiveFrameDataSize,
                    _parameters.getMaxBlockSize(),
                    _parameters.getEncodedMinSeparationTime(),
                    firstFrameCopy,
                    blocked);
                _messageReceivers.push_back(*messageReceiver);
            }
            handleTransitions(
                *messageReceiver, handleTransition(*messageReceiver), "firstDataFrameReceived");
        }
        else
        {
            char formatBuffer[FORMAT_BUFFER_SIZE];
            ::util::logger::Logger::error(
                _loggerComponent,
                "DoCanReceiver(%s)::firstDataFrameReceived(%s): Received %d bytes, "
                "expected max %d bytes!",
                getName(),
                _addressConverter.formatDataLinkAddress(
                    connection.getDataLinkAddressPair().getReceptionAddress(), formatBuffer),
                data.size(),
                _maxFirstFrameDataSize);
        }
    }
    else
    {
        char formatBuffer[FORMAT_BUFFER_SIZE];
        ::util::logger::Logger::warn(
            _loggerComponent,
            "DoCanReceiver(%s)::firstDataFrameReceived(%s): No empty message receiver "
            "found!",
            getName(),
            _addressConverter.formatDataLinkAddress(
                connection.getDataLinkAddressPair().getReceptionAddress(), formatBuffer));
    }
}

template<class DataLinkLayer>
void DoCanReceiver<DataLinkLayer>::consecutiveDataFrameReceived(
    DataLinkAddressType const receptionAddress,
    uint8_t const sequenceNumber,
    ::estd::slice<uint8_t const> const& data)
{
    MessageReceiverType* const messageReceiver = findMessageReceiver(receptionAddress);
    if ((messageReceiver == nullptr) || (!messageReceiver->isConsecutiveFrameExpected()))
    {
        char formatBuffer[FORMAT_BUFFER_SIZE];
        ::util::logger::Logger::warn(
            _loggerComponent,
            "DoCanReceiver(%s)::consecutiveDataFrameReceived(%s): unexpected consecutive frame!",
            getName(),
            _addressConverter.formatDataLinkAddress(receptionAddress, formatBuffer));
        return;
    }

    FrameSizeType const expectedSize = messageReceiver->getExpectedConsecutiveFrameDataSize();
    if (data.size() < expectedSize)
    {
        char formatBuffer[FORMAT_BUFFER_SIZE];
        ::util::logger::Logger::warn(
            _loggerComponent,
            "DoCanReceiver(%s)::consecutiveDataFrameReceived(%s): consecutive frame length "
            "(%d) less than expected (%d)!",
            getName(),
            _addressConverter.formatDataLinkAddress(receptionAddress, formatBuffer),
            data.size(),
            expectedSize);
        return;
    }

    RemoveGuard const guard(this);
    handleTransitions(
        *messageReceiver,
        messageReceiver->consecutiveFrameReceived(sequenceNumber, expectedSize, data),
        "consecutiveDataFrameReceived");
    return;
}

template<class DataLinkLayer>
void DoCanReceiver<DataLinkLayer>::cyclicTask(uint32_t const nowUs)
{
    {
        RemoveGuard const guard(this);
        for (typename MessageReceiverListType::iterator it = _messageReceivers.begin();
             it != _messageReceivers.end();
             ++it)
        {
            if (!it->updateTimer(nowUs))
            {
                break;
            }
            ::interrupts::SuspendResumeAllInterruptsScopedLock const lock;
            handleTransitions(*it, it->expired(), "cyclicTask");
        }
    }
    // Sort after removal.
    if (_timersUpdated)
    {
        ::interrupts::SuspendResumeAllInterruptsScopedLock const lock;
        _messageReceivers.sort();
        _timersUpdated = false;
    }
}

template<class DataLinkLayer>
void DoCanReceiver<DataLinkLayer>::transportMessageProcessed(
    ::transport::TransportMessage& transportMessage, ProcessingResult const /*result*/)
{
    _messageProvidingListener.releaseTransportMessage(transportMessage);
    ::async::execute(_context, _processMessageReceivers);
}

template<class DataLinkLayer>
void DoCanReceiver<DataLinkLayer>::processMessageReceivers()
{
    RemoveGuard const guard(this);
    for (typename MessageReceiverListType::iterator it = _messageReceivers.begin();
         it != _messageReceivers.end();
         ++it)
    {
        MessageReceiverType& messageReceiver = *it;
        ::interrupts::SuspendResumeAllInterruptsScopedLock const lock;
        if (messageReceiver.isAllocating())
        {
            handleTransitions(
                messageReceiver,
                allocateTransportMessage(messageReceiver, true),
                "processMessageReceivers");
            if (messageReceiver.getMessage() == nullptr)
            {
                break;
            }
        }
    }
}

template<class DataLinkLayer>
void DoCanReceiver<DataLinkLayer>::handleTransitions(
    MessageReceiverType& messageReceiver, ReceiveResult result, char const* const functionName)
{
    while (result.hasTransition())
    {
        handleResult(messageReceiver, result, functionName);
        result = handleTransition(messageReceiver);
    }
}

template<class DataLinkLayer>
void DoCanReceiver<DataLinkLayer>::handleResult(
    MessageReceiverType& messageReceiver,
    ReceiveResult const result,
    char const* const functionName)
{
    if (result.hasTransition())
    {
        resetTimer(messageReceiver);
        if (messageReceiver.getState() != ReceiveState::DONE)
        {
            _timersUpdated = true;
        }
    }
    if (result.getMessage() != ReceiveMessage::NONE)
    {
        char stringBuffer[FORMAT_BUFFER_SIZE];
        auto const formattedAddress = _addressConverter.formatDataLinkAddress(
            messageReceiver.getReceptionAddress(), stringBuffer);
        // create a log for each separately to allow compiler to skip them if no logs are
        // defined
        switch (result.getMessage())
        {
            case ReceiveMessage::ILLEGAL_STATE:
            {
                ::util::logger::Logger::warn(
                    _loggerComponent,
                    "DoCanReceiver(%s)::%s(%s): Illegal state 0x%x!",
                    getName(),
                    functionName,
                    formattedAddress,
                    result.getParam());
                break;
            }
            case ReceiveMessage::ALLOCATION_RETRY_COUNT_EXCEEDED:
            {
                ::util::logger::Logger::warn(
                    _loggerComponent,
                    "DoCanReceiver(%s)::%s(%s): Allocation retry count exceeded",
                    getName(),
                    functionName,
                    formattedAddress);
                break;
            }
            case ReceiveMessage::RX_TIMEOUT_EXPIRED:
            {
                ::util::logger::Logger::warn(
                    _loggerComponent,
                    "DoCanReceiver(%s)::%s(%s): Rx timeout",
                    getName(),
                    functionName,
                    formattedAddress);
                break;
            }
            case ReceiveMessage::BAD_SEQUENCE_NUMBER:
            {
                ::util::logger::Logger::warn(
                    _loggerComponent,
                    "DoCanReceiver(%s)::%s(%s): Frame with bad sequence number "
                    "received",
                    getName(),
                    functionName,
                    formattedAddress);
                break;
            }
            case ReceiveMessage::PROCESSING_FAILED:
            {
                ::util::logger::Logger::warn(
                    _loggerComponent,
                    "DoCanReceiver(%s)::%s(%s): Processing failed",
                    getName(),
                    functionName,
                    formattedAddress);
                break;
            }
            default:
            {
                break;
            }
        }
    }
}

template<class DataLinkLayer>
ReceiveResult DoCanReceiver<DataLinkLayer>::handleTransition(MessageReceiverType& messageReceiver)
{
    switch (messageReceiver.getState())
    {
        case ReceiveState::ALLOCATE:
        {
            return allocateTransportMessage(messageReceiver, false);
        }
        case ReceiveState::SEND:
        {
            return sendFlowControlFrame(messageReceiver);
        }
        case ReceiveState::PROCESSING:
        {
            return startProcessingTransportMessage(messageReceiver);
        }
        case ReceiveState::DONE:
        {
            return release(messageReceiver);
        }
        default:
        {
            break;
        }
    }
    return ReceiveResult(false);
}

template<class DataLinkLayer>
ReceiveResult DoCanReceiver<DataLinkLayer>::allocateTransportMessage(
    MessageReceiverType& messageReceiver, bool const peek)
{
    DoCanTransportAddressPair const transportAddressPair
        = messageReceiver.getTransportAddressPair();
    ::transport::TransportMessage* message = nullptr;
    if (!messageReceiver.isBlocked())
    {
        ::transport::ITransportMessageProvider::ErrorCode const result
            = _messageProvidingListener.getTransportMessage(
                _busId,
                transportAddressPair.getSourceId(),
                transportAddressPair.getTargetId(),
                messageReceiver.getMessageSize(),
                messageReceiver.getFirstFrameData(),
                message);
        if ((result != ::transport::ITransportMessageProvider::ErrorCode::TPMSG_OK)
            && (result
                != ::transport::ITransportMessageProvider::ErrorCode::TPMSG_NO_MSG_AVAILABLE))
        {
            char stringBuffer[FORMAT_BUFFER_SIZE];
            auto const formattedAddress = _addressConverter.formatDataLinkAddress(
                messageReceiver.getReceptionAddress(), stringBuffer);
            switch (result)
            {
                case ::transport::ITransportMessageProvider::ErrorCode::TPMSG_INVALID_SRC_ID:
                {
                    ::util::logger::Logger::warn(
                        _loggerComponent,
                        "DoCanReceiver(%s)::allocateTransportMessage(%s): Illegal "
                        "source "
                        "id.",
                        getName(),
                        formattedAddress);
                    break;
                }
                case ::transport::ITransportMessageProvider::ErrorCode::TPMSG_INVALID_TGT_ID:
                {
                    ::util::logger::Logger::warn(
                        _loggerComponent,
                        "DoCanReceiver(%s)::allocateTransportMessage(%s): Illegal "
                        "target "
                        "id.",
                        getName(),
                        formattedAddress);
                    break;
                }
                default:
                {
                    ::util::logger::Logger::warn(
                        _loggerComponent,
                        "DoCanReceiver(%s)::allocateTransportMessage(%s): "
                        "No buffer available (error: 0x%x). Message discarded.",
                        getName(),
                        formattedAddress,
                        result);
                    break;
                }
            }
            return messageReceiver.cancel();
        }

        if (message != nullptr)
        {
            message->resetValidBytes();
            message->setSourceId(transportAddressPair.getSourceId());
            message->setTargetId(transportAddressPair.getTargetId());
            message->setPayloadLength(messageReceiver.getMessageSize());
        }
        // Don't do anything in this scope if message == nullptr
    }
    if ((message == nullptr) && peek)
    {
        return ReceiveResult(false);
    }

    return messageReceiver.allocated(message, _parameters.getMaxAllocateRetryCount());
}

template<class DataLinkLayer>
ReceiveResult
DoCanReceiver<DataLinkLayer>::sendFlowControlFrame(MessageReceiverType& messageReceiver)
{
    bool success;
    if (messageReceiver.isFlowControlWait())
    {
        success = _flowControlFrameTransmitter.sendFlowControl(
            messageReceiver.getFrameCodec(),
            messageReceiver.getTransmissionAddress(),
            FlowStatus::WAIT,
            0U,
            0U);
    }
    else
    {
        success = _flowControlFrameTransmitter.sendFlowControl(
            messageReceiver.getFrameCodec(),
            messageReceiver.getTransmissionAddress(),
            FlowStatus::CTS,
            messageReceiver.getMaxBlockSize(),
            messageReceiver.getEncodedMinSeparationTime());
    }
    return messageReceiver.frameSent(success);
}

template<class DataLinkLayer>
ReceiveResult
DoCanReceiver<DataLinkLayer>::startProcessingTransportMessage(MessageReceiverType& messageReceiver)
{
    ::transport::TransportMessage& message = *messageReceiver.detachMessage();
    bool const success
        = (_messageProvidingListener.messageReceived(_busId, message, this)
           == ::transport::ITransportMessageListener::ReceiveResult::RECEIVED_NO_ERROR);
    if (!success)
    {
        _messageProvidingListener.releaseTransportMessage(message);
    }
    return messageReceiver.processed(success);
}

template<class DataLinkLayer>
ReceiveResult DoCanReceiver<DataLinkLayer>::release(MessageReceiverType& messageReceiver)
{
    DataLinkAddressType const receptionAddress   = messageReceiver.getReceptionAddress();
    ::transport::TransportMessage* const message = messageReceiver.release();
    if (message != nullptr)
    {
        _messageProvidingListener.releaseTransportMessage(*message);
    }
    for (typename MessageReceiverListType::iterator it = _messageReceivers.begin();
         it != _messageReceivers.end();
         ++it)
    {
        if (it->isBlocked() && (it->getReceptionAddress() == receptionAddress))
        {
            it->setBlocked(false);
            break;
        }
    }
    // Ensure we don't wrap _releasedReceiverCount back to 0
    estd_assert(
        _releasedReceiverCount != std::numeric_limits<decltype(_releasedReceiverCount)>::max());
    ++_releasedReceiverCount;
    return ReceiveResult(false);
}

template<class DataLinkLayer>
void DoCanReceiver<DataLinkLayer>::resetTimer(MessageReceiverType& messageReceiver) const
{
    auto const nowUs = _parameters.nowUs();
    switch (messageReceiver.getTimeout())
    {
        case ReceiveTimeout::ALLOCATE:
        {
            messageReceiver.setTimer(
                nowUs + (static_cast<uint32_t>(_parameters.getWaitAllocateTimeout()) * 1000U));
            break;
        }
        case ReceiveTimeout::RX:
        {
            messageReceiver.setTimer(
                nowUs + (static_cast<uint32_t>(_parameters.getWaitRxTimeout()) * 1000U));
            break;
        }
        default:
        {
            messageReceiver.setTimer(nowUs);
            break;
        }
    }
}

template<class DataLinkLayer>
bool DoCanReceiver<DataLinkLayer>::handlePendingMessageReceivers(
    DataLinkAddressType const receptionAddress)
{
    bool blocked = false;
    for (typename MessageReceiverListType::iterator it = _messageReceivers.begin();
         it != _messageReceivers.end();
         ++it)
    {
        if (it->getReceptionAddress() == receptionAddress)
        {
            if (it->getFrameCount() > 1U)
            {
                char formatBuffer[FORMAT_BUFFER_SIZE];

                ::util::logger::Logger::info(
                    _loggerComponent,
                    "DoCanReceiver(%s)::handlingPendingMessageReceivers(%s): "
                    "Segmented transfer cancelled due to new first frame.",
                    getName(),
                    _addressConverter.formatDataLinkAddress(
                        it->getReceptionAddress(), formatBuffer));
                handleTransitions(*it, it->cancel(), "handlingPendingMessageReceivers");
            }
            else
            {
                blocked = true;
            }
        }
    }
    return blocked;
}

template<class DataLinkLayer>
typename DoCanReceiver<DataLinkLayer>::MessageReceiverType*
DoCanReceiver<DataLinkLayer>::findMessageReceiver(DataLinkAddressType const receptionAddress)
{
    for (typename MessageReceiverListType::iterator it = _messageReceivers.begin();
         it != _messageReceivers.end();
         ++it)
    {
        if (it->getReceptionAddress() == receptionAddress)
        {
            return &*it;
        }
    }
    return nullptr;
}

template<class DataLinkLayer>
void DoCanReceiver<DataLinkLayer>::setRemoveLock()
{
    // Don't allow _removeLockCount to wrap around
    estd_assert(_removeLockCount != std::numeric_limits<decltype(_removeLockCount)>::max());
    ::interrupts::SuspendResumeAllInterruptsScopedLock const lock;
    ++_removeLockCount;
}

template<class DataLinkLayer>
void DoCanReceiver<DataLinkLayer>::releaseRemoveLock()
{
    // Don't allow _removeLockCount to wrap around
    estd_assert(_removeLockCount != 0);
    ::interrupts::SuspendResumeAllInterruptsScopedLock const lock;
    --_removeLockCount;
    if ((_removeLockCount == 0U) && (_releasedReceiverCount > 0U))
    {
        typename MessageReceiverListType::iterator prevIt = _messageReceivers.before_begin();
        typename MessageReceiverListType::iterator it     = _messageReceivers.begin();
        while (_releasedReceiverCount > 0U)
        {
            if (it->getState() == ReceiveState::DONE)
            {
                MessageReceiverType& messageReceiver = *it;
                it                                   = _messageReceivers.erase_after(prevIt);
                _messageReceiverPool.release(messageReceiver);
                --_releasedReceiverCount;
            }
            else
            {
                prevIt = it;
                ++it;
            }
        }
    }
}

template<class DataLinkLayer>
inline char const* DoCanReceiver<DataLinkLayer>::getName() const
{
    return ::common::busid::BusIdTraits::getName(_busId);
}

template<class DataLinkLayer>
inline ::estd::slice<uint8_t const> DoCanReceiver<DataLinkLayer>::copyFirstFrameData(
    uint8_t* const receiverAddress, ::estd::slice<uint8_t const> const& firstFrameData)
{
    ::estd::slice<uint8_t> firstFrameDataCopy = ::estd::slice<uint8_t>::from_pointer(
        receiverAddress + sizeof(MessageReceiverType), firstFrameData.size());
    (void)::estd::memory::copy(firstFrameDataCopy, firstFrameData);
    return firstFrameDataCopy;
}

template<class DataLinkLayer>
inline DoCanReceiver<DataLinkLayer>::RemoveGuard::RemoveGuard(DoCanReceiver* const receiver)
: _receiver(receiver)
{
    receiver->setRemoveLock();
}

template<class DataLinkLayer>
inline DoCanReceiver<DataLinkLayer>::RemoveGuard::~RemoveGuard()
{
    _receiver->releaseRemoveLock();
}

} // namespace docan
