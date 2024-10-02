// Copyright 2024 Accenture.

#ifndef GUARD_A6DC0BE7_E06D_4B81_A90E_E3127700529C
#define GUARD_A6DC0BE7_E06D_4B81_A90E_E3127700529C

#include <estd/forward_list.h>
#include <estd/uncopyable.h>
#include <platform/estdint.h>

namespace ethernet
{

class IEthernetDevice;

namespace port
{
static uint8_t const ANY_PORT     = 254U;
static uint8_t const UNKNOWN_PORT = 255U;
} // namespace port

/**
 * Interface for classes interested in ethernet link status.
 */
class ILinkStatusListener
: public ::estd::forward_list_node<ILinkStatusListener>
, public ::estd::uncopyable
{
protected:
    ILinkStatusListener();

public:
    /**
     * Link status codes
     */
    enum LinkStatus
    {
        /** No link */
        LINK_DOWN = 0U,
        /** Link is up */
        LINK_UP,
        /** Link information is not available */
        LINK_INVALID,
        /** Ethernet port is not available */
        PORT_UNAVAILABLE
    };

    /**
     * Notifies about a change in the status of a ethernet link.
     */
    virtual void linkStatusChanged(IEthernetDevice* pDevice, LinkStatus linkStatus, uint8_t port)
        = 0;
};

/*
 * inline methods
 */

inline ILinkStatusListener::ILinkStatusListener()
: ::estd::forward_list_node<ILinkStatusListener>(), ::estd::uncopyable()
{}

} // namespace ethernet

#endif /* GUARD_A6DC0BE7_E06D_4B81_A90E_E3127700529C */
