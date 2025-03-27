// Copyright 2024 Accenture.

/**
 * Contains the interface for errorlisteners for ICanTransceivers.
 * \file ICANTransceiverStateListener.h
 * \ingroup can
 */
#pragma once

namespace can
{
class ICanTransceiver;

/**
 * Interface for a class that wants to be notified about errors that occur
 * at a attached ICanTransceiver.
 *
 *
 * \see ICanTransceiver::setErrorListener()
 */
class ICANTransceiverStateListener
{
public:
    /**
     * All possible states of a CANTransceiver.
     */
    enum class CANTransceiverState : uint8_t
    {
        ACTIVE,
        PASSIVE,
        BUS_OFF
    };

    /**
     * Callback that gets called when an state change occurs at a transceiver
     * this listener is attached to.
     * \param transceiver    ICanTransceiver which reports the error
     * \param state    new can transceiver state.
     */
    virtual void canTransceiverStateChanged(ICanTransceiver& transceiver, CANTransceiverState state)
        = 0;

    /**
     * Callback that gets called when an error occurs at a transceiver
     * this listener is attached to.
     * \param transceiver    ICanTransceiver which reports the error
     */
    virtual void phyErrorOccurred(ICanTransceiver& transceiver) = 0;
};

} // namespace can
