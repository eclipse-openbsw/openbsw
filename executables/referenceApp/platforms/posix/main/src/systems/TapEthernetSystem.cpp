// Copyright 2025 Accenture.

#include "systems/TapEthernetSystem.h"

#include "ethConfig.h"

#include <poll.h>

namespace systems
{

TapEthernetSystem::TapEthernetSystem(::async::ContextType const context)
: _context(context), _rxTimeout(), _driver(ethX::MAC_ADDRESS)
{}

void TapEthernetSystem::init() { transitionDone(); }

void TapEthernetSystem::run()
{
    if (!_driver.start("tap0"))
    {
        ::util::logger::Logger::critical(
            ::util::logger::ETHERNET, "TapEthernetDriver start failed!");
    }
    else
    {
        ::util::logger::Logger::info(::util::logger::ETHERNET, "TapEthernetDriver started!");
        ::async::scheduleAtFixedRate(
            _context, *this, _rxTimeout, 1, ::async::TimeUnitType::MILLISECONDS);
    }
    transitionDone();
}

void TapEthernetSystem::shutdown()
{
    _rxTimeout.cancel();
    transitionDone();
}

void TapEthernetSystem::execute()
{
    pollfd pollFds[1];
    pollFds[0].fd     = _driver.getTapInterfaceFd();
    pollFds[0].events = POLLIN;
    while (poll(pollFds, 1, 0) > 0)
    {
        if (pollFds[0].revents & POLLIN)
        {
            _driver.readFrame();
        }
    }
}

bool TapEthernetSystem::getLinkStatus(size_t const /*port*/) { return true; }

bool TapEthernetSystem::writeFrame(netif* /*ni*/, pbuf* pb) { return _driver.writeFrame(pb); }

} // namespace systems
