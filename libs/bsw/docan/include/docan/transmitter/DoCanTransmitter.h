// Copyright 2024 Accenture.

#pragma once

#include "docan/addressing/IDoCanAddressConverter.h"
#include "docan/common/DoCanConstants.h"
#include "docan/common/DoCanParameters.h"
#include "docan/datalink/DoCanFrameCodec.h"
#include "docan/datalink/IDoCanDataFrameTransmitter.h"
#include "docan/datalink/IDoCanDataFrameTransmitterCallback.h"
#include "docan/transmitter/DoCanMessageTransmitter.h"
#include "docan/transmitter/IDoCanTickGenerator.h"

#include <async/Async.h>
#include <async/Types.h>
#include <async/util/MemberCall.h>
#include <common/busid/BusId.h>
#include <interrupts/SuspendResumeAllInterruptsScopedLock.h>
#include <transport/AbstractTransportLayer.h>
#include <util/estd/derived_object_pool.h>
#include <util/logger/Logger.h>

#include <estd/forward_list.h>

#include <algorithm>
#include <limits>

namespace docan
{
/**
 * Class responsible for parallel transmission of transport messages.
 * \tparam DataLinkLayer class providing data link functionality
 */
template<class DataLinkLayer>
class DoCanTransmitter : private IDoCanDataFrameTransmitterCallback<DataLinkLayer>
{
public:
    using DataLinkLayerType          = DataLinkLayer;
    using DataLinkAddressType        = typename DataLinkLayerType::AddressType;
    using DataLinkAddressPairType    = typename DataLinkLayerType::AddressPairType;
    using MessageSizeType            = typename DataLinkLayerType::MessageSizeType;
    using FrameIndexType             = typename DataLinkLayerType::FrameIndexType;
    using FrameSizeType              = typename DataLinkLayerType::FrameSizeType;
    using JobHandleType              = typename DataLinkLayerType::JobHandleType;
    using FrameCodecType             = DoCanFrameCodec<DataLinkLayer>;
    using DataFrameTransmitterType   = IDoCanDataFrameTransmitter<DataLinkLayerType>;
    using MessageTransmitterType     = DoCanMessageTransmitter<DataLinkLayerType>;
    using MessageTransmitterListType = ::estd::forward_list<MessageTransmitterType>;
    using MessageTransmitterListIterator =
        typename ::estd::forward_list<MessageTransmitterType>::iterator;

    /** Constructor.
     *
     * \param busId bus Id of transmitter. Will be used for logging
     *
     * \param dataFrameTransmitter reference to frame transmitter interface
     *
     * \param tickGenerator reference to tick generator interface
     *
     * \param messageTransmitterBlockPool reference to block pool to use for allocating message
     * transmitters
     *
     * \param parameters reference to object holding parameters for the transport layers. No copy is
     * held within the transport layer!
     */
    DoCanTransmitter(
        uint8_t busId,
        ::async::ContextType,
        DataFrameTransmitterType& dataFrameTransmitter,
        IDoCanTickGenerator& tickGenerator,
        ::util::estd::block_pool& messageTransmitterBlockPool,
        IDoCanAddressConverter<DataLinkLayerType>& addressConverter,
        DoCanParameters const& parameters,
        uint8_t const loggerComponent);

    /**
     * Initializes the transmitter.
     */
    void init();

    /**
     * Performs a shut down. The shut down is synchronously, all pending message transmission
     * will be aborted.
     */
    void shutdown();

    /**
     * Send a transport message.
     * \param dataLinkAddressPair reference to the pair of data link addresses
     *        for sending frames (transmission address) and receiving of flow control frames
     *        (reception address)
     * \param message reference to message to send
     * \param notificationListener optional notification listener that will be stored with
     *        the transport message
     */
    ::transport::AbstractTransportLayer::ErrorCode send(
        ::transport::TransportMessage& message,
        ::transport::ITransportMessageProcessedListener* notificationListener);

    /**
     * Called to indicate reception of a flow control frame.
     * \param receptionAddress reception address of the frame
     * \param flowStatus flow status as defined
     * \param blockSize block size
     * \param encodedMinSeparationTime encoded separation time
     */
    void flowControlFrameReceived(
        DataLinkAddressType receptionAddress,
        FlowStatus flowStatus,
        uint8_t blockSize,
        uint8_t encodedMinSeparationTime);

    /*
     * Check if any message transmitters are ready to send their next frame, or if a timeout has
     * occurred
     */
    void cyclicTask(uint32_t nowUs);

    /*
     * Check if any message transmitters are currently sending consecutive frames
     */
    bool isSendingConsecutiveFrames() const;

private:
    static uint8_t const FORMAT_BUFFER_SIZE = 32U;

    void processMessageTransmitters();

    class RemoveGuard
    {
    public:
        explicit RemoveGuard(DoCanTransmitter* transmitter);
        RemoveGuard(DoCanTransmitter* transmitter, bool remove);
        ~RemoveGuard();

    private:
        DoCanTransmitter* _transmitter;
        bool _remove;
    };

    void dataFramesSent(
        JobHandleType jobHandle, FrameIndexType frameCount, MessageSizeType dataSize) override;

    void sendNextFrames();
    void handleResult(
        MessageTransmitterType& messageTransmitter,
        TransmitResult result,
        char const* functionName);
    void resetTimer(MessageTransmitterType& messageTransmitter);

    MessageTransmitterListIterator
    findMessageTransmitterByReceptionAddress(DataLinkAddressType receptionAddress);
    MessageTransmitterListIterator findMessageTransmitterByJobHandle(JobHandleType jobHandle);

    MessageTransmitterListIterator setSendLock();
    void releaseSendLock(bool success);

    void setRemoveLock();
    void releaseRemoveLock(bool remove);

    char const* getName() const;

    IDoCanAddressConverter<DataLinkLayerType>& _addressConverter;
    ::util::estd::derived_object_pool<MessageTransmitterType> _messageTransmitterPool;
    ::async::MemberCall<DoCanTransmitter, &DoCanTransmitter::processMessageTransmitters>
        _processMessageTransmitters;
    MessageTransmitterListType _messageTransmitters;
    DataFrameTransmitterType& _dataFrameTransmitter;
    IDoCanTickGenerator& _tickGenerator;
    MessageTransmitterListIterator _sendMessageTransmitterIt;
    DoCanParameters const& _parameters;
    typename JobHandleType::CounterType _jobCounter;
    ::async::ContextType const _context;
    uint8_t const _busId;
    uint8_t const _loggerComponent;
    uint8_t _removeLockCount;
    uint8_t _releasedTransmitterCount;
    // number of DoCanMessageTransmitters currently sending consecutive frames
    uint8_t _sendingConsecutiveFramesCount;
    bool _sendLock;
    bool _pendingSend;
    bool _switchContext;
    bool _timersUpdated;
};

/**
 * Inline implementation.
 */
template<class DataLinkLayer>
DoCanTransmitter<DataLinkLayer>::DoCanTransmitter(
    uint8_t const busId,
    ::async::ContextType const context,
    DataFrameTransmitterType& dataFrameTransmitter,
    IDoCanTickGenerator& tickGenerator,
    ::util::estd::block_pool& messageTransmitterBlockPool,
    IDoCanAddressConverter<DataLinkLayerType>& addressConverter,
    DoCanParameters const& parameters,
    uint8_t const loggerComponent)
: _addressConverter(addressConverter)
, _messageTransmitterPool(messageTransmitterBlockPool)
, _processMessageTransmitters(*this)
, _messageTransmitters()
, _dataFrameTransmitter(dataFrameTransmitter)
, _tickGenerator(tickGenerator)
, _sendMessageTransmitterIt(_messageTransmitters.end())
, _parameters(parameters)
, _jobCounter(0U)
, _context(context)
, _busId(busId)
, _loggerComponent(loggerComponent)
, _removeLockCount(0U)
, _releasedTransmitterCount(0U)
, _sendingConsecutiveFramesCount(0U)
, _sendLock(false)
, _pendingSend(false)
, _switchContext(false)
, _timersUpdated(false)
{}

template<class DataLinkLayer>
inline void DoCanTransmitter<DataLinkLayer>::init()
{
    _jobCounter               = 0U;
    _sendMessageTransmitterIt = _messageTransmitters.end();
}

template<class DataLinkLayer>
void DoCanTransmitter<DataLinkLayer>::shutdown()
{
    RemoveGuard const guard(this);
    for (MessageTransmitterListIterator it = _messageTransmitters.begin();
         it != _messageTransmitters.end();
         ++it)
    {
        ::interrupts::SuspendResumeAllInterruptsScopedLock const lock;
        handleResult(*it, it->cancel(), "shutdown");
    }
}

template<class DataLinkLayer>
::transport::AbstractTransportLayer::ErrorCode DoCanTransmitter<DataLinkLayer>::send(
    ::transport::TransportMessage& message,
    ::transport::ITransportMessageProcessedListener* const notificationListener)
{
    DoCanTransportAddressPair const transportAddressPair(
        message.getSourceId(), message.getTargetId());
    DataLinkAddressPairType dataLinkAddressPair;
    DoCanFrameCodec<DataLinkLayer> const* const codec
        = _addressConverter.getTransmissionParameters(transportAddressPair, dataLinkAddressPair);
    if (codec == nullptr)
    {
        ::util::logger::Logger::warn(
            _loggerComponent,
            "DoCanTransmitter(%s)::send(0x%x -> 0x%x): invalid source/target pair.",
            getName(),
            message.getSourceId(),
            message.getTargetId());
        return ::transport::AbstractTransportLayer::ErrorCode::TP_SEND_FAIL;
    }

    if (!message.isComplete())
    {
        return ::transport::AbstractTransportLayer::ErrorCode::TP_MESSAGE_INCOMPLETE;
    }
    FrameIndexType frameCount{};
    FrameSizeType consecutiveFrameDataSize;
    if (codec->getEncodedFrameCount(
            message.getPayloadLength(), frameCount, consecutiveFrameDataSize)
        != CodecResult::OK)
    {
        return ::transport::AbstractTransportLayer::ErrorCode::TP_GENERAL_ERROR;
    }
    ::interrupts::SuspendResumeAllInterruptsScopedLock const lock;
    if ((frameCount > 1U)
        && (findMessageTransmitterByReceptionAddress(dataLinkAddressPair.getReceptionAddress())
            != _messageTransmitters.end()))
    {
        ::util::logger::Logger::warn(
            _loggerComponent,
            "DoCanTransmitter(%s)::send(0x%x -> 0x%x): Already a segmented message for this "
            "source/target pair!",
            getName(),
            message.getSourceId(),
            message.getTargetId());
        return ::transport::AbstractTransportLayer::ErrorCode::TP_SEND_FAIL;
    }
    if (_messageTransmitterPool.empty())
    {
        char formatBuffer[FORMAT_BUFFER_SIZE];
        ::util::logger::Logger::warn(
            _loggerComponent,
            "DoCanTransmitter(%s)::send(%s): No empty message transmitter found!",
            getName(),
            _addressConverter.formatDataLinkAddress(
                dataLinkAddressPair.getReceptionAddress(), formatBuffer));
        return ::transport::AbstractTransportLayer::ErrorCode::TP_QUEUE_FULL;
    }
    ::estd::constructor<MessageTransmitterType> ctor
        = _messageTransmitterPool.template allocate<MessageTransmitterType>();
    auto const poolIndex = static_cast<typename JobHandleType::UserDataType>(
        _messageTransmitterPool.get_block_pool().index_of(static_cast<uint8_t*>(ctor)));
    JobHandleType const jobHandle(++_jobCounter, poolIndex);
    MessageTransmitterType& messageTransmitter = ctor.construct(
        jobHandle,
        *codec,
        dataLinkAddressPair,
        ::estd::by_ref(message),
        notificationListener,
        frameCount,
        consecutiveFrameDataSize);
    _messageTransmitters.push_back(messageTransmitter);

    ::async::execute(_context, _processMessageTransmitters);
    return ::transport::AbstractTransportLayer::ErrorCode::TP_OK;
}

template<class DataLinkLayer>
void DoCanTransmitter<DataLinkLayer>::dataFramesSent(
    JobHandleType const jobHandle, FrameIndexType const frameCount, MessageSizeType const dataSize)
{
    RemoveGuard const guard(this, false);
    ::interrupts::SuspendResumeAllInterruptsScopedLock const lock;
    _pendingSend = false;

    MessageTransmitterListIterator const messageTransmitter
        = findMessageTransmitterByJobHandle(jobHandle);
    if (messageTransmitter != _messageTransmitters.end())
    {
        handleResult(
            *messageTransmitter,
            messageTransmitter->framesSent(frameCount, dataSize),
            "dataFramesSent");
        if (isSendingConsecutiveFrames())
        {
            _tickGenerator.tickNeeded();
        }
    }
}

template<class DataLinkLayer>
void DoCanTransmitter<DataLinkLayer>::flowControlFrameReceived(
    DataLinkAddressType const receptionAddress,
    FlowStatus const flowStatus,
    uint8_t const blockSize,
    uint8_t const encodedMinSeparationTime)
{
    RemoveGuard const guard(this);
    ::interrupts::SuspendResumeAllInterruptsScopedLock const lock;
    MessageTransmitterListIterator const messageTransmitter
        = findMessageTransmitterByReceptionAddress(receptionAddress);
    if (messageTransmitter == _messageTransmitters.end())
    {
        char formatBuffer[FORMAT_BUFFER_SIZE];
        ::util::logger::Logger::warn(
            _loggerComponent,
            "DoCanTransmitter(%s)::flowControlFrameReceived(%s): no message pending to be sent!",
            getName(),
            _addressConverter.formatDataLinkAddress(receptionAddress, formatBuffer));
        return;
    }
    handleResult(
        *messageTransmitter,
        messageTransmitter->handleFlowControl(
            flowStatus,
            blockSize,
            DoCanParameters::decodeMinSeparationTime(encodedMinSeparationTime),
            _parameters.getMaxFlowControlWaitCount()),
        "flowControlFrameReceived");
}

template<class DataLinkLayer>
inline bool DoCanTransmitter<DataLinkLayer>::isSendingConsecutiveFrames() const
{
    return _sendingConsecutiveFramesCount > 0U;
}

template<class DataLinkLayer>
void DoCanTransmitter<DataLinkLayer>::cyclicTask(uint32_t const nowUs)
{
    {
        RemoveGuard const guard(this);
        for (MessageTransmitterListIterator it = _messageTransmitters.begin();
             it != _messageTransmitters.end();
             ++it)
        {
            ::interrupts::SuspendResumeAllInterruptsScopedLock const lock;
            if (!it->updateTimer(nowUs))
            {
                break;
            }
            handleResult(*it, it->expired(), "cyclicTask");
        }
    }
    if (_timersUpdated)
    {
        ::interrupts::SuspendResumeAllInterruptsScopedLock const lock;
        _messageTransmitters.sort();
        _timersUpdated = false;
    }
}

template<class DataLinkLayer>
void DoCanTransmitter<DataLinkLayer>::processMessageTransmitters()
{
    RemoveGuard const guard(this);
    for (MessageTransmitterListIterator it = _messageTransmitters.begin();
         it != _messageTransmitters.end();
         ++it)
    {
        if (it->getState() == TransmitState::INITIALIZED)
        {
            ::interrupts::SuspendResumeAllInterruptsScopedLock const lock;
            handleResult(*it, it->start(), "processMessageTransmitters");
        }
    }
}

template<class DataLinkLayer>
void DoCanTransmitter<DataLinkLayer>::sendNextFrames()
{
    while (true)
    {
        // attempt to set a send lock
        auto const sendMessageTransmitter = setSendLock();

        if (sendMessageTransmitter != _messageTransmitters.end())
        {
            MessageTransmitterType& messageTransmitter = *sendMessageTransmitter;
            FrameIndexType const blockEnd = (messageTransmitter.getMinSeparationTimeUs() == 0U)
                                                ? messageTransmitter.getBlockEnd()
                                                : (messageTransmitter.getFrameIndex() + 1U);
            SendResult const result       = _dataFrameTransmitter.startSendDataFrames(
                messageTransmitter.getFrameCodec(),
                *this,
                messageTransmitter.getJobHandle(),
                messageTransmitter.getTransmissionAddress(),
                messageTransmitter.getFrameIndex(),
                blockEnd,
                messageTransmitter.getConsecutiveFrameDataSize(),
                messageTransmitter.getSendData());
            if ((result == SendResult::QUEUED) || (result == SendResult::QUEUED_FULL))
            {
                _pendingSend = (result == SendResult::QUEUED_FULL);
                handleResult(
                    messageTransmitter, messageTransmitter.frameSending(), "sendNextFrame");
                releaseSendLock(true);
                continue;
            }
            if (result == SendResult::FAILED)
            {
                handleResult(messageTransmitter, messageTransmitter.cancel(), "sendNextFrames");
            }
            // otherwise, sending will be retried from cyclicTask
        }
        releaseSendLock(false);
        break;
    }
}

template<class DataLinkLayer>
void DoCanTransmitter<DataLinkLayer>::handleResult(
    MessageTransmitterType& messageTransmitter,
    TransmitResult const result,
    char const* const functionName)
{
    if (result.hasTransition())
    {
        resetTimer(messageTransmitter);
        if (messageTransmitter.isDone())
        {
            if (result.getActionSet().test(TransmitAction::CANCEL_SEND))
            {
                _dataFrameTransmitter.cancelSendDataFrames(
                    *this, messageTransmitter.getJobHandle());
                _pendingSend = false;
            }
            messageTransmitter.release();
            // Ensure we don't wrap _releasedTransmitterCount back to 0
            estd_assert(
                _releasedTransmitterCount
                != std::numeric_limits<decltype(_releasedTransmitterCount)>::max());
            ++_releasedTransmitterCount;
            _switchContext = true;
        }
        else
        {
            _timersUpdated = true;
        }
    }
    if (result.getMessage() != TransmitMessage::NONE)
    {
        // create a log for each separately to allow compiler to skip them if no logs are defined
        ::transport::TransportMessage& message = messageTransmitter.getMessage();
        switch (result.getMessage())
        {
            case TransmitMessage::ILLEGAL_STATE:
            {
                ::util::logger::Logger::warn(
                    _loggerComponent,
                    "DoCanTransmitter(%s)::%s(0x%x -> 0x%x): Illegal state 0x%x!",
                    getName(),
                    functionName,
                    message.getSourceId(),
                    message.getTargetId(),
                    result.getParam());
                break;
            }
            case TransmitMessage::TX_CALLBACK_TIMEOUT_EXPIRED:
            {
                ::util::logger::Logger::warn(
                    _loggerComponent,
                    "DoCanTransmitter(%s)::%s(0x%x -> 0x%x): Tx callback timeout",
                    getName(),
                    functionName,
                    message.getSourceId(),
                    message.getTargetId());
                break;
            }
            case TransmitMessage::FLOW_CONTROL_TIMEOUT_EXPIRED:
            {
                ::util::logger::Logger::warn(
                    _loggerComponent,
                    "DoCanTransmitter(%s)::%s(0x%x -> 0x%x): Flow control timeout",
                    getName(),
                    functionName,
                    message.getSourceId(),
                    message.getTargetId());
                break;
            }
            case TransmitMessage::FLOW_CONTROL_INVALID:
            {
                ::util::logger::Logger::warn(
                    _loggerComponent,
                    "DoCanTransmitter(%s)::%s(0x%x -> 0x%x): Invalid flow control received",
                    getName(),
                    functionName,
                    message.getSourceId(),
                    message.getTargetId());
                break;
            }
            case TransmitMessage::FLOW_CONTROL_OVERFLOW:
            {
                ::util::logger::Logger::warn(
                    _loggerComponent,
                    "DoCanTransmitter(%s)::%s(0x%x -> 0x%x): Flow control overflow received",
                    getName(),
                    functionName,
                    message.getSourceId(),
                    message.getTargetId());
                break;
            }
            case TransmitMessage::FLOW_CONTROL_WAIT_COUNT_EXCEEDED:
            {
                ::util::logger::Logger::warn(
                    _loggerComponent,
                    "DoCanTransmitter(%s)::%s(0x%x -> 0x%x): Flow control wait count exceeded",
                    getName(),
                    functionName,
                    message.getSourceId(),
                    message.getTargetId());
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
void DoCanTransmitter<DataLinkLayer>::resetTimer(MessageTransmitterType& messageTransmitter)
{
    auto const nowUs = _parameters.nowUs();

    if (messageTransmitter.isSendingConsecutiveFrames())
    {
        // Ensure _tickNeededCount won't wrap around
        estd_assert(_sendingConsecutiveFramesCount != 0);
        --_sendingConsecutiveFramesCount;
    }

    switch (messageTransmitter.getTimeout())
    {
        case TransmitTimeout::TX_CALLBACK:
        {
            messageTransmitter.setTimer(
                nowUs + static_cast<uint32_t>(_parameters.getWaitTxCallbackTimeout()) * 1000U);
            break;
        }
        case TransmitTimeout::FLOW_CONTROL:
        {
            messageTransmitter.setTimer(
                nowUs + static_cast<uint32_t>(_parameters.getWaitFlowControlTimeout()) * 1000U);
            break;
        }
        case TransmitTimeout::SEPARATION_TIME:
        {
            messageTransmitter.setTimer(nowUs + messageTransmitter.getMinSeparationTimeUs(), true);
            break;
        }
        default:
        {
            messageTransmitter.setTimer(nowUs);
            break;
        }
    }

    if (messageTransmitter.isSendingConsecutiveFrames())
    {
        // Ensure _tickNeededCount won't wrap around
        estd_assert(
            _sendingConsecutiveFramesCount
            != std::numeric_limits<decltype(_sendingConsecutiveFramesCount)>::max());
        ++_sendingConsecutiveFramesCount;
    }
}

template<class DataLinkLayer>
typename DoCanTransmitter<DataLinkLayer>::MessageTransmitterListIterator
DoCanTransmitter<DataLinkLayer>::findMessageTransmitterByReceptionAddress(
    DataLinkAddressType const receptionAddress)
{
    for (auto&& it = _messageTransmitters.begin(); it != _messageTransmitters.end(); ++it)
    {
        if (receptionAddress == it->getReceptionAddress())
        {
            return it;
        }
    }
    return _messageTransmitters.end();
}

template<class DataLinkLayer>
typename DoCanTransmitter<DataLinkLayer>::MessageTransmitterListIterator
DoCanTransmitter<DataLinkLayer>::findMessageTransmitterByJobHandle(JobHandleType const jobHandle)
{
    for (auto&& it = _messageTransmitters.begin(); it != _messageTransmitters.end(); ++it)
    {
        if (jobHandle == it->getJobHandle())
        {
            return it;
        }
    }
    return _messageTransmitters.end();
}

template<class DataLinkLayer>
typename DoCanTransmitter<DataLinkLayer>::MessageTransmitterListIterator
DoCanTransmitter<DataLinkLayer>::setSendLock()
{
    ::interrupts::SuspendResumeAllInterruptsScopedLock const lock;
    if ((_sendLock) || (_pendingSend) || (_messageTransmitters.empty()))
    {
        return _messageTransmitters.end();
    }

    if (_sendMessageTransmitterIt == _messageTransmitters.end())
    {
        _sendMessageTransmitterIt = _messageTransmitters.begin();
    }
    MessageTransmitterListIterator const startMessageTransmitterIt = _sendMessageTransmitterIt;

    do
    {
        if (_sendMessageTransmitterIt->getState() == TransmitState::SEND)
        {
            _sendLock = true;
            return _sendMessageTransmitterIt;
        }
        ++_sendMessageTransmitterIt;
        if (_sendMessageTransmitterIt == _messageTransmitters.end())
        {
            _sendMessageTransmitterIt = _messageTransmitters.begin();
        }
    } while (_sendMessageTransmitterIt != startMessageTransmitterIt);
    return _messageTransmitters.end();
}

template<class DataLinkLayer>
void DoCanTransmitter<DataLinkLayer>::releaseSendLock(bool const success)
{
    ::interrupts::SuspendResumeAllInterruptsScopedLock const lock;
    if (success && (_sendMessageTransmitterIt != _messageTransmitters.end()))
    {
        ++_sendMessageTransmitterIt;
    }
    _sendLock = false;
}

template<class DataLinkLayer>
void DoCanTransmitter<DataLinkLayer>::setRemoveLock()
{
    // Ensure the _removeLockCount increment won't wrap around
    estd_assert(_removeLockCount != std::numeric_limits<decltype(_removeLockCount)>::max());
    ::interrupts::SuspendResumeAllInterruptsScopedLock const lock;
    ++_removeLockCount;
}

template<class DataLinkLayer>
void DoCanTransmitter<DataLinkLayer>::releaseRemoveLock(bool const remove)
{
    sendNextFrames();
    bool switchContext;
    MessageTransmitterListType removedTransmitters;
    MessageTransmitterListIterator prevRemovedIt = removedTransmitters.before_begin();
    {
        // Ensure the _removeLockCount decrement won't wrap around
        estd_assert(_removeLockCount != 0);
        ::interrupts::SuspendResumeAllInterruptsScopedLock const lock;
        --_removeLockCount;
        if (remove && (_removeLockCount == 0U))
        {
            MessageTransmitterListIterator prevIt = _messageTransmitters.before_begin();
            MessageTransmitterListIterator it     = _messageTransmitters.begin();
            while (_releasedTransmitterCount > 0U)
            {
                if (it->isDone())
                {
                    if (it == _sendMessageTransmitterIt)
                    {
                        _pendingSend = false;

                        ++_sendMessageTransmitterIt;
                    }
                    MessageTransmitterType& messageTransmitter = *it;
                    it = _messageTransmitters.erase_after(prevIt);
                    prevRemovedIt
                        = removedTransmitters.insert_after(prevRemovedIt, messageTransmitter);
                    --_releasedTransmitterCount;
                }
                else
                {
                    prevIt = it;
                    ++it;
                }
            }
        }
        switchContext  = _switchContext;
        _switchContext = false;
    }
    while (!removedTransmitters.empty())
    {
        MessageTransmitterType& messageTransmitter = removedTransmitters.front();
        removedTransmitters.pop_front();
        if (messageTransmitter.getNotificationListener() != nullptr)
        {
            using ::transport::ITransportMessageProcessedListener;
            auto processingResult
                = ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR;
            if (messageTransmitter.getState() != TransmitState::SUCCESS)
            {
                auto const message = messageTransmitter.getErrorMessage();
                if (message == TransmitMessage::FLOW_CONTROL_TIMEOUT_EXPIRED)
                {
                    processingResult = ITransportMessageProcessedListener::ProcessingResult::
                        PROCESSED_ERROR_TIMEOUT;
                }
                else if (message == TransmitMessage::FLOW_CONTROL_OVERFLOW)
                {
                    processingResult = ITransportMessageProcessedListener::ProcessingResult::
                        PROCESSED_ERROR_OVERFLOW;
                }
                else if (message == TransmitMessage::FLOW_CONTROL_INVALID)
                {
                    processingResult = ITransportMessageProcessedListener::ProcessingResult::
                        PROCESSED_ERROR_ABORT;
                }
                else
                {
                    processingResult = ITransportMessageProcessedListener::ProcessingResult::
                        PROCESSED_ERROR_GENERAL;
                }
            }
            messageTransmitter.getNotificationListener()->transportMessageProcessed(
                messageTransmitter.getMessage(), processingResult);
        }
        ::interrupts::SuspendResumeAllInterruptsScopedLock const lock;
        _messageTransmitterPool.release(messageTransmitter);
    }
    if (switchContext)
    {
        ::async::execute(_context, _processMessageTransmitters);
    }
}

template<class DataLinkLayer>
inline char const* DoCanTransmitter<DataLinkLayer>::getName() const
{
    return ::common::busid::BusIdTraits::getName(_busId);
}

template<class DataLinkLayer>
inline DoCanTransmitter<DataLinkLayer>::RemoveGuard::RemoveGuard(
    DoCanTransmitter* const transmitter)
: RemoveGuard(transmitter, true)
{}

template<class DataLinkLayer>
inline DoCanTransmitter<DataLinkLayer>::RemoveGuard::RemoveGuard(
    DoCanTransmitter* const transmitter, bool const remove)
: _transmitter(transmitter), _remove(remove)
{
    transmitter->setRemoveLock();
}

template<class DataLinkLayer>
inline DoCanTransmitter<DataLinkLayer>::RemoveGuard::~RemoveGuard()
{
    _transmitter->releaseRemoveLock(_remove);
}

} // namespace docan

