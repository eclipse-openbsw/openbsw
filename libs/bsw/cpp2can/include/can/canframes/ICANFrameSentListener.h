// Copyright 2024 Accenture.

/**
 * Contains interface ICANFrameSentListener.
 * \file    ICANFrameSentListener.h
 * \ingroup    canframes
 */
#ifndef GUARD_9001E931_0C39_4A49_83A4_60726128845A
#define GUARD_9001E931_0C39_4A49_83A4_60726128845A

namespace can
{
class CANFrame;

/**
 * Interface for listeners that need to be notified when a CANFrame has
 * been sent.
 *
 */
class ICANFrameSentListener
{
protected:
    ICANFrameSentListener() = default;

public:
    ICANFrameSentListener(ICANFrameSentListener const&)            = delete;
    ICANFrameSentListener& operator=(ICANFrameSentListener const&) = delete;
    /**
     * \warning
     * This callback is called from ISR context!
     */
    virtual void canFrameSent(CANFrame const& frame)               = 0;
};

} // namespace can

#endif /*GUARD_9001E931_0C39_4A49_83A4_60726128845A*/
