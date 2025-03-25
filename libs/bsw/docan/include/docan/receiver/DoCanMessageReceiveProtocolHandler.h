// Copyright 2024 Accenture.

#pragma once

#include <platform/estdint.h>

namespace docan
{
/**
 * Enumeration defining state of the message.
 */
enum class ReceiveState : uint8_t
{
    /// A transport message should be allocated.
    ALLOCATE,
    /// Waiting for receiving a frame or next try for message allocation.
    WAIT,
    /// A flow control frame should be sent.
    SEND,
    /// The received message should be processed.
    PROCESSING,
    /// The reception of the message has ended.
    DONE
};

/**
 * Enumeration defining timeouts to be set.
 */
enum class ReceiveTimeout : uint8_t
{
    /// No timeout is needed.
    NONE,
    /// Timeout for reception of a frame is needed.
    RX,
    /// Timeout until next allocation of message is needed.
    ALLOCATE
};

/**
 * Message state emitted by the receiver.
 */
enum class ReceiveMessage : uint8_t
{
    /// No message.
    NONE,
    /// An event has occurred in an unexpected state.
    ILLEGAL_STATE,
    /// The maximum number of allocation retries has exceeded.
    ALLOCATION_RETRY_COUNT_EXCEEDED,
    /// The timeout for frame reception has expired.
    RX_TIMEOUT_EXPIRED,
    /// A frame with an unexpected sequence number has been received.
    BAD_SEQUENCE_NUMBER,
    /// The processing of the message was not successful.
    PROCESSING_FAILED
};

/**
 * Helper class that holds the result of event handling. Most important
 * is the hasTransition() method that indicates whether a state transition
 * to a new state has been performed.
 */
class ReceiveResult
{
public:
    /**
     * constructor. No message and no parameter is set.
     * \param transition flag whether a transition has been performed
     */
    explicit ReceiveResult(bool transition);

    /**
     * Set a message without parameter.
     * \param message message to emit
     * \return reference to this object
     */
    ReceiveResult& setMessage(ReceiveMessage message);

    /**
     * Set a message with parameter.
     * \param message message to emit
     * \param param additional parameter value
     * \return reference to this object
     */
    ReceiveResult& setMessage(ReceiveMessage message, uint8_t param);

    /**
     * Test whether a transition has occurred. A state transition indicates that
     * timeouts should be reset.
     * \return true if a transition has occurred.
     */
    bool hasTransition() const { return _transition; }

    /**
     * Get the message that is connected with the result.
     * \return message value
     */
    ReceiveMessage getMessage() const { return _message; }

    /**
     * Get the parameter to a message.
     * \return parameter value
     */
    uint8_t getParam() const { return _param; }

    /**
     * Check result for equality. Results are equal if all fields are identical.
     * \param other result to compare with
     * \return true if result is equal to other
     */
    bool operator==(ReceiveResult const& other) const;

private:
    bool _transition;
    ReceiveMessage _message;
    uint8_t _param;
};

/**
 * Class encapsulating the protocol handling for reception of a single message.
 */
template<typename FrameIndexType>
class DoCanMessageReceiveProtocolHandler
{
public:
    /**
     * Constructor.
     * \param frameCount number of expected frames
     */
    explicit DoCanMessageReceiveProtocolHandler(FrameIndexType frameCount);

    /**
     * Get the current state of the message.
     * \return state
     */
    ReceiveState getState() const { return _state; }

    /**
     * Get the timeout to be set for this state.
     * \return timeout
     */
    ReceiveTimeout getTimeout() const { return _timeout; }

    /**
     * Get the index of the next expected frame.
     * \return frame index
     */
    FrameIndexType getFrameIndex() const { return _frameIndex; }

    /**
     * Get the number of expected frames for the message.
     * \return frame count
     */
    FrameIndexType getFrameCount() const { return _frameCount; }

    /**
     * Test whether the a wait flow control frame should be sent.
     * This flag is only valid if state is STATE_SEND.
     * \return true if the next flow control frame should be a wait frame
     */
    bool isFlowControlWait() const { return (_allocateRetryCount > 0U) && (_frameCount > 1U); }

    /**
     * Test whether message allocation is required. This "allocation" state
     * can be present in more than one state.
     * \return true if allocation of a message is required
     */
    bool isAllocating() const { return _isAllocating; }

    /**
     * Cancel the reception process and set to state done.
     * \param message message to emit
     * \return result indicating state transition (and given message)
     */
    ReceiveResult cancel(ReceiveMessage const message = ReceiveMessage::NONE)
    {
        return setDone(message);
    }

    /**
     * Handles a shutdown request during message reception.
     * \return result indicating state transition
     */
    ReceiveResult shutdown()
    {
        return (_state == ReceiveState::PROCESSING) ? ReceiveResult(false) : cancel();
    }

    /**
     * Called to indicate the result of a message allocation try.
     * \param success true if a message has been allocated successfully
     * \param maxRetryCount maximum number of retries
     * \return result indicating state transition
     */
    ReceiveResult allocated(bool success, uint8_t maxRetryCount);

    /**
     * Called to indicate that the CAN frame has been sent.
     * \param success true if the CAN frame has been sent successfully
     * \return result indicating state transition
     */
    ReceiveResult frameSent(bool success);

    /**
     * Called to indicate that a consecutive frame has been received.
     * \param frameIndex sequence number of the received frame
     * \param maxBlockSize maximum block size for message reception
     * \return result indicating state transition
     */
    ReceiveResult consecutiveFrameReceived(uint8_t frameIndex, uint8_t maxBlockSize);

    /**
     * Called to indicate the result of message processing.
     * \param success true if message processing was successful
     * \return result indicating state transition
     */
    ReceiveResult processed(bool const success)
    {
        return setDone(success ? ReceiveMessage::NONE : ReceiveMessage::PROCESSING_FAILED);
    }

    /**
     * Called to indicate that the current timeout has expired
     * \return result indicating state transition
     */
    ReceiveResult expired()
    {
        switch (_timeout)
        {
            case ReceiveTimeout::RX:
            {
                return setDone(ReceiveMessage::RX_TIMEOUT_EXPIRED);
            }
            case ReceiveTimeout::ALLOCATE:
            {
                return setState(ReceiveState::ALLOCATE);
            }
            default:
            {
                return ReceiveResult(false);
            }
        }
    }

private:
    inline ReceiveResult
    setState(ReceiveState state, ReceiveTimeout timeout = ReceiveTimeout::NONE);
    inline ReceiveResult setDone(ReceiveMessage message = ReceiveMessage::NONE, uint8_t param = 0U);

    FrameIndexType _frameIndex;
    FrameIndexType const _frameCount;
    ReceiveState _state;
    ReceiveTimeout _timeout;
    uint8_t _blockFrameIndex;
    uint8_t _allocateRetryCount;
    bool _isAllocating;
};

/**
 * Inline implementation.
 */

template<typename FrameIndexType>
inline DoCanMessageReceiveProtocolHandler<FrameIndexType>::DoCanMessageReceiveProtocolHandler(
    FrameIndexType const frameCount)
: _frameIndex(1U)
, _frameCount(frameCount)
, _state(ReceiveState::ALLOCATE)
, _timeout(ReceiveTimeout::NONE)
, _blockFrameIndex(0U)
, _allocateRetryCount(0U)
, _isAllocating(true)
{}

template<typename FrameIndexType>
inline ReceiveResult DoCanMessageReceiveProtocolHandler<FrameIndexType>::allocated(
    bool const success, uint8_t const maxRetryCount)
{
    if (!_isAllocating)
    {
        return ReceiveResult(true).setMessage(
            ReceiveMessage::ILLEGAL_STATE, static_cast<uint8_t>(_state));
    }

    if (success)
    {
        _isAllocating       = false;
        _allocateRetryCount = 0U;
        return setState((_frameCount == 1U) ? ReceiveState::PROCESSING : ReceiveState::SEND);
    }

    if (_frameCount == 1U)
    {
        ++_allocateRetryCount;
        if (_allocateRetryCount > 1U)
        {
            return setDone(ReceiveMessage::ALLOCATION_RETRY_COUNT_EXCEEDED);
        }

        return setState(ReceiveState::WAIT, ReceiveTimeout::ALLOCATE);
    }

    ++_allocateRetryCount;
    if (_allocateRetryCount > maxRetryCount)
    {
        return setDone(ReceiveMessage::ALLOCATION_RETRY_COUNT_EXCEEDED);
    }

    return setState(ReceiveState::SEND);
}

template<typename FrameIndexType>
inline ReceiveResult
DoCanMessageReceiveProtocolHandler<FrameIndexType>::frameSent(bool const success)
{
    if (_state != ReceiveState::SEND)
    {
        return ReceiveResult(true).setMessage(
            ReceiveMessage::ILLEGAL_STATE, static_cast<uint8_t>(_state));
    }
    if (success)
    {
        return setState(
            ReceiveState::WAIT,
            isFlowControlWait() ? ReceiveTimeout::ALLOCATE : ReceiveTimeout::RX);
    }
    return ReceiveResult(false);
}

template<typename FrameIndexType>
inline ReceiveResult DoCanMessageReceiveProtocolHandler<FrameIndexType>::consecutiveFrameReceived(
    uint8_t const frameIndex, uint8_t const maxBlockSize)
{
    if ((_state != ReceiveState::WAIT) || (_timeout != ReceiveTimeout::RX))
    {
        return ReceiveResult(false);
    }

    if ((_frameIndex & 0xFU) != frameIndex)
    {
        return setDone(ReceiveMessage::BAD_SEQUENCE_NUMBER, frameIndex);
    }

    ++_frameIndex;
    if (_frameIndex == _frameCount)
    {
        return setState(ReceiveState::PROCESSING);
    }
    if (maxBlockSize > 0U)
    {
        ++_blockFrameIndex;
        if (_blockFrameIndex == maxBlockSize)
        {
            _blockFrameIndex = 0U;
            return setState(ReceiveState::SEND);
        }
    }
    // set state to wait for timeout as in the default case
    return setState(ReceiveState::WAIT, ReceiveTimeout::RX);
}

template<typename FrameIndexType>
inline ReceiveResult DoCanMessageReceiveProtocolHandler<FrameIndexType>::setState(
    ReceiveState const state, ReceiveTimeout const timeout)
{
    _state   = state;
    _timeout = timeout;
    return ReceiveResult(true);
}

template<typename FrameIndexType>
inline ReceiveResult DoCanMessageReceiveProtocolHandler<FrameIndexType>::setDone(
    ReceiveMessage const message, uint8_t const param)
{
    _state        = ReceiveState::DONE;
    _timeout      = ReceiveTimeout::NONE;
    _isAllocating = false;
    return ReceiveResult(true).setMessage(message, param);
}

inline ReceiveResult::ReceiveResult(bool const transition)
: _transition(transition), _message(ReceiveMessage::NONE), _param()
{}

inline ReceiveResult& ReceiveResult::setMessage(ReceiveMessage const message)
{
    _message = message;
    _param   = 0U;
    return *this;
}

inline ReceiveResult& ReceiveResult::setMessage(ReceiveMessage const message, uint8_t const param)
{
    _message = message;
    _param   = param;
    return *this;
}

inline bool ReceiveResult::operator==(ReceiveResult const& other) const
{
    return (_transition == other._transition) && (_message == other._message)
           && (_param == other._param);
}

} // namespace docan

