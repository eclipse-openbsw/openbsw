// Copyright 2024 Accenture.

/**
 * Contains class AbstractCANTransceiver.
 * \file         AbstractCANTransceiver.h
 * \ingroup        transceiver
 */
#pragma once

#include "can/canframes/CANFrame.h"
#include "can/framemgmt/AbstractBitFieldFilteredCANFrameListener.h"
#include "can/framemgmt/AbstractIntervalFilteredCANFrameListener.h"
#include "can/framemgmt/IFilteredCANFrameSentListener.h"
#include "can/transceiver/ICANTransceiverStateListener.h"
#include "can/transceiver/ICanTransceiver.h"

#include <etl/intrusive_forward_list.h>
#include <etl/intrusive_list.h>

#include <platform/estdint.h>

namespace can
{
class ICANFrameSentListener;
class IFilteredCANFrameSentListener;
class FrameDispatcher;
class CanTransportLayer;

/**
 * Cpp2CAN AbstractCANTransceiver
 * \class        AbstractCANTransceiver
 *
 *
 * An AbstractCANTransceiver is the common hardware independent base class for
 * CAN Transceiver classes. This base implementation or the ICanTransceiver interface
 * shall be used in all applications to avoid HW dependencies.
 *
 * This abstract class implements the HW independent methods.
 *
 * \section        Reception
 * An implementation of AbstractCANTransceiver has to fill a CANFrame object
 * with the data of the received can frame and then call
 * notifyListeners() with the received CANFrame which distributes it to all
 * listeners, i.e. to all listeners whose filter match the id of fRxFrame.
 *
 */
class AbstractCANTransceiver : public ICanTransceiver
{
public:
    /**
     * constructor
     * \param    busId    busId of MultiChannelCanTransceiver
     * \post    getState() == CLOSED
     */
    explicit AbstractCANTransceiver(uint8_t busId);

    /**
     * \return    internal state of transceiver
     * \see    State
     */
    State getState() const override;

    /**
     * adds an ICANFrameListener to listener list
     * \param    canFrameListener    listener to register
     *
     * This method contains a critical section and uses Suspend-/ResumeOSInterrupts.
     *
     * \attention
     * The filter of canFrameListener is merged into the transceivers rx-filter.
     * It must be configured before adding the listener!
     */
    void addCANFrameListener(ICANFrameListener& listener) override;

    /**
     * Adds a listener to the front of the receiver list.
     * \see    addCANFrameListener
     * Using this method makes sure that the added listener is notified first
     * of a incoming CANFrame.
     */
    void addVIPCANFrameListener(ICANFrameListener& listener) override;

    /**
     * removes an ICANFrameListener from listener list
     * \param    canFrameListener    listener to remove
     *
     * This method contains a critical section and uses Suspend-/ResumeOSInterrupts.
     *
     * \note
     * The elements of canFrameListeners filter will not be removed from
     * the transceivers rx-filter.
     */
    void removeCANFrameListener(ICANFrameListener& listener) override;

    /**
     * adds an addCANFrameSentListener to listener list
     * \param    listener    listener to register
     *
     * This method contains a critical section and uses Suspend-/ResumeOSInterrupts.
     *
     * \attention
     * The filter of listener is merged into the transceivers rx-filter.
     * It must be configured before adding the listener!
     */
    void addCANFrameSentListener(IFilteredCANFrameSentListener& listener) override;

    /**
     * removes an ICANFrameListener from listener list
     * \param    listener    listener to remove
     *
     * This method contains a critical section and uses Suspend-/ResumeOSInterrupts.
     *
     * \note
     * The elements of listener filter will not be removed from
     * the transceivers rx-filter.
     */
    void removeCANFrameSentListener(IFilteredCANFrameSentListener& listener) override;

    /**
     * \return    busId of transceiver
     */
    uint8_t getBusId() const override { return _busId; }

    /**
     * Get the hardware state of the CAN transceiver.
     * \return    state    hardware state of the CAN transceiver
     */
    ICANTransceiverStateListener::CANTransceiverState getCANTransceiverState() const override
    {
        return _transceiverState;
    }

    /**
     * Sets the transceivers ICANTransceiverStateListener.
     * \param    pListener    ICANTransceiverStateListener to notify when an error
     * occurs. 0L if no class needs to be notified.
     */
    void setStateListener(ICANTransceiverStateListener& listener) override
    {
        _stateListener = &listener;
    }

    void removeStateListener() override { _stateListener = nullptr; }

protected:
    void setState(State newState);

    bool isInState(State state) const;

    /**
     * Notifies all appropriate listeners about the reception of a CANFrame.
     * \param    frame    CANFrame that will be passed to all listeners
     */
    void notifyListeners(CANFrame const& frame);

    void notifySentListeners(can::CANFrame const& frame);

    /**
     * Notifies the attached ICANTransceiverStateListener that a phy error occurred
     */
    void notifyStateListenerWithPhyError()
    {
        if (_stateListener != nullptr)
        {
            _stateListener->phyErrorOccurred(*this);
        }
    }

    /**
     * Notifies the attached ICANTransceiverStateListener with a given
     * state
     * \param    state    new can transceiver state.
     */
    void notifyStateListenerWithState(ICANTransceiverStateListener::CANTransceiverState state);

protected:
    BitFieldFilter _filter;
    ::etl::intrusive_list<ICANFrameListener, ::etl::bidirectional_link<0>> _listeners;
    IFilteredCANFrameSentListener* _sentListener;
    ::etl::intrusive_forward_list<IFilteredCANFrameSentListener, ::etl::forward_link<0>>
        _sentListeners;
    /** baudrate of transceiver */
    uint32_t _baudrate;
    /** internal State */
    State _state;
    uint8_t _busId;
    ICANTransceiverStateListener* _stateListener;
    ICANTransceiverStateListener::CANTransceiverState _transceiverState;
};

inline ICanTransceiver::State AbstractCANTransceiver::getState() const { return _state; }

inline void AbstractCANTransceiver::setState(ICanTransceiver::State const newState)
{
    _state = newState;
}

inline bool AbstractCANTransceiver::isInState(ICanTransceiver::State const state) const
{
    return (_state == state);
}

inline void AbstractCANTransceiver::notifyStateListenerWithState(
    ICANTransceiverStateListener::CANTransceiverState const state)
{
    if (_stateListener != nullptr)
    {
        _stateListener->canTransceiverStateChanged(*this, state);
    }
}

} // namespace can
