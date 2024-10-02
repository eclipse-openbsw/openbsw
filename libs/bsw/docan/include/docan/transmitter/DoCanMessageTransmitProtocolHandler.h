// Copyright 2024 Accenture.

#ifndef GUARD_E4A42D8B_E35A_4785_AE36_695D4B303FB8
#define GUARD_E4A42D8B_E35A_4785_AE36_695D4B303FB8

#include "docan/common/DoCanConstants.h"

#include <estd/bitset.h>
#include <platform/estdint.h>

namespace docan
{

/**
 * Enumeration defining tranmission states.
 */
enum class TransmitState : uint8_t
{
    /// The handler is initialized and waits for signal to start.
    INITIALIZED,
    /// A can frame should be sent.
    SEND,
    /// The handler waits for an event.
    WAIT,
    /// The transmission has completed.
    SUCCESS,
    /// The transmission has failed.
    FAIL
};

/**
 * Enumeration defining timeouts to be set.
 */
enum class TransmitTimeout : uint8_t
{
    /// No timeout needed.
    NONE,
    /// Timeout for transceiver callback is needed.
    TX_CALLBACK,
    /// Timeout for expecting a received flow control frame is needed.
    FLOW_CONTROL,
    /// Timeout for sending the next frame is needed.
    SEPARATION_TIME
};

/**
 * Messages emitted by the handler.
 */
enum class TransmitMessage : uint8_t
{
    /// No message.
    NONE,
    /// An event has occurred in an unexpected state.
    ILLEGAL_STATE,
    /// The wait time for the transmission callback of the last frame has expired.
    TX_CALLBACK_TIMEOUT_EXPIRED,
    /// The maximum number of wait control frames has been sent.
    FLOW_CONTROL_WAIT_COUNT_EXCEEDED,
    /// A overflow control frame has been received.
    FLOW_CONTROL_OVERFLOW,
    /// The wait timeout for the reception of the next control frame has expired.
    FLOW_CONTROL_TIMEOUT_EXPIRED,
    /// An invalid control frame has been received.
    FLOW_CONTROL_INVALID,
};

/**
 * Action flags.
 */
struct TransmitAction
{
    /// action flag indicating that the separation time needs to be stored.
    static uint8_t const STORE_SEPARATION_TIME = 0U;
    /// action flag indicating that a pending send should be canceled.
    static uint8_t const CANCEL_SEND           = 1U;
    /// number of actions flags.
    static uint8_t const COUNT                 = 2U;
};

using TransmitActionSetType = ::estd::bitset<TransmitAction::COUNT>;

/**
 * Helper class that holds the result of event handling. Most important
 * is the hasTransition() method that indicates whether a state transition
 * to a new state has been performed.
 */
class TransmitResult
{
public:
    /**
     * Constructor for indicating an event result without message.
     * \param transition true if a state transition has been performed
     */
    explicit TransmitResult(bool transition);

    /**
     * Set additional actions that should be performed.
     * \param actionSet set of bits indicating actions to perform
     * \return reference to this object
     */
    TransmitResult& setActionSet(TransmitActionSetType actionSet);

    /**
     * Set message.
     * \param message message value
     * \return reference to this object
     */
    TransmitResult& setMessage(TransmitMessage message);

    /**
     * Set message with additional parameter
     * \param message message value
     * \param param parameter value
     * \return reference to this object
     */
    TransmitResult& setMessage(TransmitMessage message, uint8_t param);

    /**
     * Set message with additional parameter
     * \param message message value
     * \param param State parameter value
     * \return reference to this object
     */
    TransmitResult& setMessage(TransmitMessage message, TransmitState param);

    /**
     * Test whether a transition has occurred. A state transition indicates that
     * timeouts should be reset.
     * \return true if a transition has occurred.
     */
    bool hasTransition() const;
    /**
     * Get the action mask.
     * \return action mask
     */
    TransmitActionSetType getActionSet() const;
    /**
     * Get the message that is connected with the result.
     * \return message value
     */
    TransmitMessage getMessage() const;
    /**
     * Get the parameter to a message.
     * \return parameter value
     */
    uint8_t getParam() const;

    /**
     * Check result for equality. Results are equal if all fields are identical.
     * \param other result to compare with
     * \return true if result is equal to other
     */
    bool operator==(TransmitResult const& other) const;

private:
    bool _transition;
    TransmitActionSetType _actionSet;
    TransmitMessage _message;
    uint8_t _param;
};

/**
 * Class encapsulating the protocol handling for transmission of a single message.
 */
template<typename FrameIndexType>
class DoCanMessageTransmitProtocolHandler
{
public:
    /**
     * Constructor.
     * \param frameCount total number of frames to send
     */
    explicit DoCanMessageTransmitProtocolHandler(FrameIndexType frameCount);

    /**
     * Get the current state of the protocol handler.
     * \return state
     */
    TransmitState getState() const { return _state; }

    /**
     * Get the timeout to be set for this state.
     * \return timeout
     */
    TransmitTimeout getTimeout() const { return _timeout; }

    /**
     * Get the frame index of the next frame to transmit.
     * \return index
     */
    FrameIndexType getFrameIndex() const { return _frameIndex; }

    /**
     * Get the index of the block end.
     * \return index of block end
     */
    FrameIndexType getBlockEnd() const { return _blockEnd; }

    /**
     * Get the total number of frames to transmit.
     * \return total number
     */
    FrameIndexType getFrameCount() const { return _frameCount; }

    /**
     * Check whether the transmission has been successful.
     * \return true if successful
     */
    bool isDone() const
    {
        return (_state == TransmitState::SUCCESS) || (_state == TransmitState::FAIL);
    }

    /**
     * Cancel the transmission process and set to failed state.
     * \param message message to emit
     * \return result indicating state transition (and given message)
     */
    TransmitResult cancel(TransmitMessage const message = TransmitMessage::NONE)
    {
        return setFailed(message);
    }

    /**
     * Start the message transmission after initialization (typically called
     * from correct task context).
     * \return result indicating state transition
     */
    TransmitResult start() { return setSend(); }

    /**
     * Indicate that sending a frame has started.
     * \return result indicating state transition
     */
    TransmitResult frameSending();

    /**
     * Indicate that one or more frames have been sent.
     * \param frameCount number of frames sent
     * \return result indicating state transition
     */
    TransmitResult framesSent(FrameIndexType frameCount);
    /**
     * Called to process a received flow control frame
     * \param flowStatus flow status field
     * \param blockSize received block size
     * \param hasMinSeparationTime true if a min separation time has been received
     * \param maxWaitCount maximum number of accepted consecutive wait frames
     * \return result indicating state transition
     */
    TransmitResult handleFlowControl(
        FlowStatus flowStatus, uint8_t blockSize, bool hasMinSeparationTime, uint8_t maxWaitCount);

    /**
     * Called to indicate that the current timeout has expired
     * \return result indicating state transition
     */
    TransmitResult expired();

    /**
     * Get the error message for this messageTransmitter.
     */
    TransmitMessage getErrorMessage() const { return _errorMessage; }

private:
    enum class FlowControl : uint8_t
    {
        UNEXPECTED,
        EXPECTED,
        RECEIVED_CTS,
        RECEIVED_WAIT

    };

    inline TransmitResult setSend();
    inline TransmitResult setState(
        TransmitState state,
        FlowControl flowControl = FlowControl::UNEXPECTED,
        TransmitTimeout timeout = TransmitTimeout::NONE);
    inline TransmitResult setFailed(TransmitMessage message, uint8_t param = 0U);

    FrameIndexType _frameIndex;
    FrameIndexType const _frameCount;
    FrameIndexType _blockEnd;
    TransmitState _state;
    TransmitTimeout _timeout;
    FlowControl _flowControl;
    TransmitMessage _errorMessage;
    uint8_t _flowControlWaitCount;
    bool _hasMinSeparationTime;
};

/**
 * inline implementation.
 */

template<typename FrameIndexType>
inline DoCanMessageTransmitProtocolHandler<FrameIndexType>::DoCanMessageTransmitProtocolHandler(
    FrameIndexType const frameCount)
: _frameIndex(0U)
, _frameCount(frameCount)
, _blockEnd(1U)
, _state(TransmitState::INITIALIZED)
, _timeout(TransmitTimeout::NONE)
, _flowControl(FlowControl::UNEXPECTED)
, _errorMessage(TransmitMessage::NONE)
, _flowControlWaitCount()
, _hasMinSeparationTime()
{}

template<typename FrameIndexType>
inline TransmitResult DoCanMessageTransmitProtocolHandler<FrameIndexType>::frameSending()
{
    if (_state == TransmitState::SEND)
    {
        FrameIndexType const nextFrameIndex = _frameIndex + 1U;
        if ((nextFrameIndex == _blockEnd) && (nextFrameIndex != _frameCount))
        {
            _flowControlWaitCount = 0U;
            return setState(
                TransmitState::WAIT, FlowControl::EXPECTED, TransmitTimeout::TX_CALLBACK);
        }
        return setState(TransmitState::WAIT, FlowControl::UNEXPECTED, TransmitTimeout::TX_CALLBACK);
    }
    return setFailed(TransmitMessage::ILLEGAL_STATE, static_cast<uint8_t>(_state));
}

template<typename FrameIndexType>
inline TransmitResult
DoCanMessageTransmitProtocolHandler<FrameIndexType>::framesSent(FrameIndexType const frameCount)
{
    if ((_state != TransmitState::WAIT) || (_timeout != TransmitTimeout::TX_CALLBACK))
    {
        return TransmitResult(true).setMessage(
            TransmitMessage::ILLEGAL_STATE, static_cast<uint8_t>(_state));
    }

    _frameIndex += frameCount;
    if (_frameIndex >= _frameCount)
    {
        return setState(TransmitState::SUCCESS);
    }

    if ((frameCount > 1U) && (_frameIndex >= _blockEnd))
    {
        _frameIndex  = _blockEnd;
        _flowControl = FlowControl::EXPECTED;
    }
    switch (_flowControl)
    {
        case FlowControl::RECEIVED_CTS:
        {
            return setSend();
        }
        case FlowControl::RECEIVED_WAIT:
        case FlowControl::EXPECTED:
        {
            return setState(TransmitState::WAIT, _flowControl, TransmitTimeout::FLOW_CONTROL);
        }
        default:
        {
            if (_hasMinSeparationTime)
            {
                return setState(
                    TransmitState::WAIT, FlowControl::UNEXPECTED, TransmitTimeout::SEPARATION_TIME);
            }
            return setSend();
        }
    }
}

template<typename FrameIndexType>
inline TransmitResult DoCanMessageTransmitProtocolHandler<FrameIndexType>::handleFlowControl(
    FlowStatus const flowStatus,
    uint8_t const blockSize,
    bool const hasMinSeparationTime,
    uint8_t const maxWaitCount)
{
    if (_flowControl != FlowControl::EXPECTED)
    {
        return TransmitResult(false);
    }

    switch (flowStatus)
    {
        case FlowStatus::CTS:
        {
            _blockEnd             = (blockSize > 0U) ? (_blockEnd + blockSize) : _frameCount;
            _hasMinSeparationTime = hasMinSeparationTime;
            if (_timeout == TransmitTimeout::FLOW_CONTROL)
            {
                (void)setSend(); // Result of setSend doesn't seem to matter, we know what to return
                return TransmitResult(true).setActionSet(
                    TransmitActionSetType().set(TransmitAction::STORE_SEPARATION_TIME));
            }

            _flowControl = FlowControl::RECEIVED_CTS;
            return TransmitResult(false).setActionSet(
                TransmitActionSetType().set(TransmitAction::STORE_SEPARATION_TIME));
        }
        case FlowStatus::WAIT:
        {
            if (_flowControlWaitCount < maxWaitCount)
            {
                ++_flowControlWaitCount;
                if (_timeout == TransmitTimeout::FLOW_CONTROL)
                {
                    return setState(
                        TransmitState::WAIT, FlowControl::EXPECTED, TransmitTimeout::FLOW_CONTROL);
                }

                _flowControl = FlowControl::RECEIVED_WAIT;
                return TransmitResult(false);
            }

            return setFailed(TransmitMessage::FLOW_CONTROL_WAIT_COUNT_EXCEEDED);
        }
        case FlowStatus::OVFLW:
        {
            return setFailed(TransmitMessage::FLOW_CONTROL_OVERFLOW);
        }
        default:
        {
            return setFailed(TransmitMessage::FLOW_CONTROL_INVALID);
        }
    }
}

template<typename FrameIndexType>
inline TransmitResult DoCanMessageTransmitProtocolHandler<FrameIndexType>::expired()
{
    switch (_timeout)
    {
        case TransmitTimeout::TX_CALLBACK:
        {
            return setFailed(TransmitMessage::TX_CALLBACK_TIMEOUT_EXPIRED);
        }
        case TransmitTimeout::FLOW_CONTROL:
        {
            return setFailed(TransmitMessage::FLOW_CONTROL_TIMEOUT_EXPIRED);
        }
        case TransmitTimeout::SEPARATION_TIME:
        {
            return setSend();
        }
        default:
        {
            return TransmitResult(false);
        }
    }
}

template<typename FrameIndexType>
inline TransmitResult DoCanMessageTransmitProtocolHandler<FrameIndexType>::setSend()
{
    _state       = TransmitState::SEND;
    _flowControl = FlowControl::UNEXPECTED;
    _timeout     = TransmitTimeout::TX_CALLBACK;
    return TransmitResult(true);
}

template<typename FrameIndexType>
inline TransmitResult DoCanMessageTransmitProtocolHandler<FrameIndexType>::setState(
    TransmitState const state, FlowControl const flowControl, TransmitTimeout const timeout)
{
    _state       = state;
    _flowControl = flowControl;
    _timeout     = timeout;
    return TransmitResult(true);
}

template<typename FrameIndexType>
inline TransmitResult DoCanMessageTransmitProtocolHandler<FrameIndexType>::setFailed(
    TransmitMessage const message, uint8_t const param)
{
    TransmitActionSetType actionSet;
    (void)actionSet.set(
        TransmitAction::CANCEL_SEND,
        (_state == TransmitState::WAIT) && (_timeout == TransmitTimeout::TX_CALLBACK));
    _state        = TransmitState::FAIL;
    _flowControl  = FlowControl::UNEXPECTED;
    _timeout      = TransmitTimeout::NONE;
    _errorMessage = message;
    return TransmitResult(true).setMessage(message, param).setActionSet(actionSet);
}

inline TransmitResult::TransmitResult(bool const transition)
: _transition(transition), _actionSet(), _message(TransmitMessage::NONE), _param()
{}

inline TransmitResult& TransmitResult::setActionSet(TransmitActionSetType const actionSet)
{
    _actionSet = actionSet;
    return *this;
}

inline TransmitResult& TransmitResult::setMessage(TransmitMessage const message)
{
    _message = message;
    _param   = 0U;
    return *this;
}

inline TransmitResult&
TransmitResult::setMessage(TransmitMessage const message, uint8_t const param)
{
    _message = message;
    _param   = param;
    return *this;
}

inline TransmitResult&
TransmitResult::setMessage(TransmitMessage const message, TransmitState const param)
{
    _message = message;
    _param   = static_cast<uint8_t>(param);
    return *this;
}

inline bool TransmitResult::hasTransition() const { return _transition; }

inline TransmitMessage TransmitResult::getMessage() const { return _message; }

inline TransmitActionSetType TransmitResult::getActionSet() const { return _actionSet; }

inline uint8_t TransmitResult::getParam() const { return _param; }

inline bool TransmitResult::operator==(TransmitResult const& other) const
{
    return (_transition == other._transition) && (_actionSet == other._actionSet)
           && (_message == other._message) && (_param == other._param);
}
} // namespace docan

#endif // GUARD_E4A42D8B_E35A_4785_AE36_695D4B303FB8
