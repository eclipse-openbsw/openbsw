// Copyright 2024 Accenture.

/**
 * Contains class ICanTransceiver.
 * \file        ICanTransceiver.h
 * \ingroup     transceiver
 */
#pragma once

#include "can/transceiver/ICANTransceiverStateListener.h"

#include <estd/forward_list.h>
#include <platform/estdint.h>

namespace can
{
class CANFrame;
class CanTransportLayer;
class FrameDispatcher;
class ICANFrameListener;
class ICANFrameSentListener;
class IFilteredCANFrameSentListener;

/**
 *  Cpp2Can Can Transceiver base interface
 *
 * \see AbstractCANTransceiver for a base HW independent implementation.
 *
 * \section Statemachine
 * A subclass of ICanTransceiver has to implement the following
 * statemachine. Initial state shall be CLOSED.
 *
 * <table border=1 bordercolor="black">
 * <tr>
 * <th>state</th><th>method called</th><th>follow state</th>
 * </tr>
 * <tr>
 * <td>CLOSED</td><td>init()</td><td>INITIALIZED</td>
 * </tr>
 * <tr>
 * <td>INITIALIZED</td><td>open()</td><td>OPEN</td>
 * </tr>
 * <tr>
 * <td>OPEN</td><td>close()</td><td>CLOSED</td>
 * </tr>
 * <tr>
 * <td>OPEN</td><td>mute()</td><td>MUTED</td>
 * </tr>
 * <tr>
 * <td>MUTED</td><td>unmute()</td><td>OPEN</td>
 * </tr>
 * <tr>
 * <td>MUTED</td><td>close()</td><td>CLOSED</td>
 * </tr>
 * </table>
 *
 *
 *
 * \todo
 * add a callback for successful transmission of a CANFrame
 */
class ICanTransceiver
{
protected:
    ICanTransceiver() = default;

public:
    ICanTransceiver(ICanTransceiver const&)            = delete;
    ICanTransceiver& operator=(ICanTransceiver const&) = delete;

    /**
     * All ErroCodes used by CanTransceivers
     */
    enum class ErrorCode : uint8_t
    {
        /** Everything OK */
        CAN_ERR_OK,
        /** Transmission failed */
        CAN_ERR_TX_FAIL,
        /** Hw queue was full */
        CAN_ERR_TX_HW_QUEUE_FULL,
        /** Transceiver was offline when trying to transmit */
        CAN_ERR_TX_OFFLINE,
        /** Transceiver is in an illegal state for the requested operation */
        CAN_ERR_ILLEGAL_STATE,
        /** The list of possible listeners is full */
        CAN_ERR_NO_MORE_LISTENERS_POSSIBLE,
        /** The requested baudrate is not supported */
        CAN_ERR_UNSUPPORTED_BAUDRATE,
        /** Transceiver could not be initalized */
        CAN_ERR_INIT_FAILED
    };

    /**
     * States used by CanTransceivers
     */
    enum class State : uint8_t
    {
        /** Transceiver is closed */
        CLOSED,
        /** Transceivers hardware is initialized */
        INITIALIZED,
        /** Transceiver is waking up bus participants */
        WAKING,
        /** Transceiver is open */
        OPEN,
        /** Transceiver does not send but receive */
        MUTED
    };

    enum
    {
        /** Size of receive queue */
        RX_QUEUE_SIZE      = 32,
        /** Lowspeed baudrate is 100 kBit/s */
        BAUDRATE_LOWSPEED  = 100000,
        /** Highspeed baudrate is 500 kBit/s */
        BAUDRATE_HIGHSPEED = 500000,
        INVALID_FRAME_ID   = 0xFFFF
    };

    /**
     * Initializes the transceivers hardware and enables the transceiver
     * to write CANFrames but not to receive them.
     * \pre     getState() == CLOSED
     * \post    getState() == INITIALIZED
     * \return
     *          - CAN_ERR_OK: transition executed
     *          - CAN_ERR_ILLEGAL_STATE: method was called in wrong state
     */
    virtual ErrorCode init() = 0;

    /**
     * Called before finally shutting down, used to clean up e.g. pending timeouts.
     */
    virtual void shutdown() = 0;

    /**
     * Sets the transceiver to a status ready for transmission and reception under the
     * restriction, that no other frame will be sent before frame frame.
     * When the frame is transmitted, the state must be set to OPEN, e.g.
     * in a tx callback function.
     * If this functionality is not required, the function shall assert to inhibit
     * unwanted usage.
     * \param frame frame to transmit during wake up phase
     * \pre     getState() == INITIALIZED
     * \post    getState() == WAKING
     * \return
     *          - CAN_ERR_OK: transition executed
     *          - CAN_ERR_ILLEGAL_STATE: method was called in wrong state
     */
    virtual ErrorCode open(CANFrame const& frame) = 0;

    /**
     * Sets the transceiver to a status ready for transmission and reception
     * \pre     getState() == INITIALIZED
     * \post    getState() == OPEN
     * \return
     *          - CAN_ERR_OK: transition executed
     *          - CAN_ERR_ILLEGAL_STATE: method was called in wrong state
     */
    virtual ErrorCode open() = 0;

    /**
     * Stops the transceiver and shuts down hardware disabling both
     * reception and transmission
     * \pre     getState() == OPEN
     * \post    getState() == CLOSED
     * \return
     *          - CAN_ERR_OK: transition executed
     *          - CAN_ERR_ILLEGAL_STATE: method was called in wrong state
     */
    virtual ErrorCode close() = 0;

    /**
     * Sets the transceiver to a state able to receive CANFrames but
     * unable to transmit
     * \pre     getState() == OPEN
     * \post    getState() == MUTED
     * \return
     *          - CAN_ERR_OK: transition executed
     *          - CAN_ERR_ILLEGAL_STATE: method was called in wrong state
     */
    virtual ErrorCode mute() = 0;

    /**
     * Enables sending CANFrames again after CANTransceiver has been muted
     * \pre     getState() == MUTED
     * \post    getState() == OPEN
     * \return
     *          - CAN_ERR_OK: transition executed
     *          - CAN_ERR_ILLEGAL_STATE: method was called in wrong state
     */
    virtual ErrorCode unmute() = 0;

    /**
     * \return  internal state of transceiver
     * \see State
     */
    virtual State getState() const = 0;

    /**
     * \return  baudrate of transceiver
     */
    virtual uint32_t getBaudrate() const = 0;

    /**
     * \return  The timeout after which the hw-queue will be empty again.
     * The value is computed from baudrate and queue depth.
     */
    virtual uint16_t getHwQueueTimeout() const = 0;

    /**
     * Writes a CANFrame to Can bus
     * \param frame frame to transmit
     * \return
     *          - CAN_ERR_OK: transmission was successful
     *          - CAN_ERR_TX_HW_QUEUE_FULL: tx-hardware-queue is full
     *          - CAN_ERR_TX_OFFLINE: transceiver was offline while trying to transmit
     *          - CAN_ERR_TX_FAIL: transmission failed with unspecified error
     *          - CAN_ERR_ILLEGAL_STATE: method was called in wrong state
     *
     * \pre     getState() == INITIALIZED || OPEN
     *
     * \section Timing
     * A call to this method is non-blocking,
     * i.e. it immediately returns an error if e.g. the hw-queue is full
     * and relies on the calling task to retry the transmission.
     */
    virtual ErrorCode write(CANFrame const& frame) = 0;

    /**
     * As other write() method with additional listener for
     * callbacks once sent.
     */
    virtual ErrorCode write(CANFrame const& frame, ICANFrameSentListener& listener) = 0;

    /**
     * Adds an ICANFrameListener to listener list
     * \param   canFrameListener    listener to register
     *
     * This method might contains a critical section and uses Suspend-/ResumeOSInterrupts.
     *
     * \attention
     * The filter of canFrameListener shall be merged into the tranceivers rx-filter.
     * It must be configured before adding the listener!
     */
    virtual void addCANFrameListener(ICANFrameListener& listener) = 0;

    /**
     * Adds a listener to the front of the receiver list.
     * \see addCANFrameListener
     * Using this method makes sure that the added listener is notified first
     * of a incoming CANFrame.
     */
    virtual void addVIPCANFrameListener(ICANFrameListener& listener) = 0;

    /**
     * Removes an ICANFrameListener from listener list
     * \param   canFrameListener    listener to remove
     *
     * This method might contain a critical section and uses Suspend-/ResumeOSInterrupts.
     *
     * \attention
     * The elements of canFrameListeners filter will not be removed from
     * the transceivers rx-filter.
     */
    virtual void removeCANFrameListener(ICANFrameListener& listener) = 0;

    /**
     * \return  busId of transceiver
     */
    virtual uint8_t getBusId() const = 0;

    /**
     * adds an addCANFrameSentListener to listener list
     * \param    listener    listener to register
     *
     * \attention
     * The filter of listener is merged into the transceivers rx-filter.
     * It must be configured before adding the listener!
     */
    virtual void addCANFrameSentListener(IFilteredCANFrameSentListener& listener) = 0;

    /**
     * removes an ICANFrameListener from listener list
     * \param    listener    listener to remove
     *
     * \note
     * The elements of listener filter will not be removed from
     * the transceivers rx-filter.
     */
    virtual void removeCANFrameSentListener(IFilteredCANFrameSentListener& listener) = 0;

    /**
     * Get the hardware state of the CAN transceiver.
     * \return  state   hardware state of the CAN transceiver
     */
    virtual ICANTransceiverStateListener::CANTransceiverState getCANTransceiverState() const = 0;

    /**
     * Sets the transceivers ICANTransceiverStateListener.
     * \param   pListener   ICANTransceiverStateListener to notify when an error
     * occurrs.
     */
    virtual void setStateListener(ICANTransceiverStateListener& listener) = 0;

    virtual void removeStateListener() = 0;

    /**
     * \note OBSOLETE INTERFACE
     *
     * Please use addCANFrameSentListener()
     *
     * Sets and ICANFrameSentListener that gets called when a frame has been
     * sent. There is only one listener possible.
     * \param pListener
     */
    virtual void setCANFrameSentListener(IFilteredCANFrameSentListener* pListener) = 0;
};

} // namespace can
