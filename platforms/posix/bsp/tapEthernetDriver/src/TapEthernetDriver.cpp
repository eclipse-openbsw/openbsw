// Copyright 2025 Accenture.

#include "TapEthernetDriver.h"

#include "lwip/arch.h"
#include "lwip/netif.h"
#include "lwip/pbuf.h"
#include "lwip/stats.h"
#include "lwipSocket/utils/LwipHelper.h"
#include "lwipopts.h"
#include "netif/etharp.h"
#include "util/estd/big_endian.h"
#if LWIP_IPV6
#include "lwip/ethip6.h"
#endif

#include <linux/if.h>
#include <linux/if_tun.h>
#include <sys/ioctl.h>

#include <fcntl.h>
#include <mutex>
#include <thread>
#include <unistd.h>

// If the name of a persistently established interface is passed (i.e. tap0), that will be used.
// Otherwise, a transient tap interface will be created.
static int allocTapInterface(char const* const ifName)
{
    char const* cloneDev = "/dev/net/tun";

    int fd = open(cloneDev, O_RDWR);
    if (fd < 0)
    {
        return -1;
    }

    ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
    if (ifName)
    {
        strncpy(ifr.ifr_name, ifName, IFNAMSIZ);
    }

    int result = ioctl(fd, TUNSETIFF, (void*)&ifr);
    if (result < 0)
    {
        close(fd);
        return -1;
    }

    return fd;
}

namespace ethernet
{

void TapEthernetDriver::setGroupcastAddressRecognition(uint8_t const* const /*mac*/) const {}

TapEthernetDriver::TapEthernetDriver(::etl::array<uint8_t const, 6> const macAddr)
: _macAddr(macAddr), _tapFd(-1)
{}

bool TapEthernetDriver::start(char const* const ifName)
{
    _tapFd = allocTapInterface(ifName);
    return _tapFd >= 0;
}

void TapEthernetDriver::stop() { _tapFd = -1; }

void TapEthernetDriver::readFrame()
{
    auto sender = ::lwiputils::PbufQueue::Sender(_queue);
    if (sender.full())
    {
        // TODO: increment queue full stat counter
        return;
    }

    auto const frameData = new uint8_t[::ethernet::TapEthernetDriver::MAX_FRAME_LENGTH];
    auto const nread     = read(_tapFd, frameData, ::ethernet::TapEthernetDriver::MAX_FRAME_LENGTH);
    if ((nread <= 0) || (nread > MAX_FRAME_LENGTH))
    {
        delete[] frameData;
        return;
    }

    auto const frameBuf = new ::lwiputils::RxCustomPbuf;

    // This lwip function is thread safe, so we can call it outside the lwip thread
    pbuf_alloced_custom(
        PBUF_RAW,
        static_cast<uint16_t>(nread),
        PBUF_REF,
        &frameBuf->buf,
        frameData,
        ::ethernet::TapEthernetDriver::MAX_FRAME_LENGTH);
    frameBuf->slot                     = frameData;
    frameBuf->buf.custom_free_function = [](pbuf* p)
    {
        // We can "upcast" here since the initial allocation was made as RxCustimPbuf
        // The pbuf is embedded as the first memeber so the address stays the same.
        auto* driverPbuf = reinterpret_cast<::lwiputils::RxCustomPbuf*>(p);
        delete[] reinterpret_cast<uint8_t*>(driverPbuf->slot);
        delete driverPbuf;
    };

    sender.write(&frameBuf->buf.pbuf);
}

bool TapEthernetDriver::writeFrame(pbuf* const buf) const
{
    if (_tapFd < 0)
    {
        return false;
    }

    uint8_t sendBuffer[MAX_FRAME_LENGTH];
    uint16_t const copiedBytes = pbuf_copy_partial(buf, sendBuffer, MAX_FRAME_LENGTH, 0U);

    if (write(_tapFd, sendBuffer, copiedBytes) <= 0)
    {
        return false;
    }

    return true;
}

} // namespace ethernet
