// Copyright 2025 Accenture.

#pragma once

#include "etl/array.h"
#include "lwipSocket/netif/LwipNetworkInterface.h"
#include "lwipSocket/utils/LwipHelper.h"
#include "util/spsc/Queue.h"

#include <cstdint>

namespace ethernet
{

static constexpr uint8_t LENGTH_ETH_ADDR = 6U;
static constexpr uint16_t TPID1          = 0x8100U;
static constexpr uint16_t VID_MASK       = 0x0FFFU;

/**
 * Driver for a TAP interface.
 *
 * \note
 * This driver currently supports one netif only.
 */
class TapEthernetDriver
{
public:
    explicit TapEthernetDriver(::etl::array<uint8_t const, 6> macAddr);

    TapEthernetDriver(TapEthernetDriver const&)            = delete;
    TapEthernetDriver& operator=(TapEthernetDriver const&) = delete;

    /**
     * Attach to a tap interface
     *
     * \param ifName Optional tap interface name to attach to (can be persistent tap interface
     * name). Can be nullptr to automatically choose.
     */
    bool start(char const* ifName);

    void stop();

    void readFrame();

    bool writeFrame(pbuf* buf) const;

    static constexpr uint8_t LENGTH_VLAN_TAG   = 4U;
    static constexpr uint16_t MAX_FRAME_LENGTH = 1518U + LENGTH_VLAN_TAG;

    /**
     * TODO: NYI
     */
    void setGroupcastAddressRecognition(::etl::array<uint8_t, 6> const mac) const;

    int getTapInterfaceFd() const { return _tapFd; }

    // Input prepared lwip pbufs to their interfaces. To be called from the lwip async context.
    void inputPbufs();

    ::etl::array<uint8_t const, LENGTH_ETH_ADDR> _macAddr;
    int _tapFd;

    // Queue of pbufs filled in the main thread and consumed in the lwip async context.
    ::lwiputils::PbufQueue _queue;
};

} // namespace ethernet
