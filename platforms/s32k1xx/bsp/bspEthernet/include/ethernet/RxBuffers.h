// Copyright 2025 Accenture.

#pragma once

#include "ethernet/EnetDevice.h"
#include "etl/span.h"
#include "lwipSocket/utils/LwipHelper.h"

namespace ethernet
{

class RxBuffers
{
public:
    uint16_t const RX_BUFFER_SIZE;

    RxBuffers(
        ::etl::span<ENET_ERXD> const descriptors,
        ::etl::span<pbuf*> const pbufAtIndex,
        ::etl::span<uint8_t> const rxBuffers,
        ::etl::span<::lwiputils::RxCustomPbuf> const customPbufs,
        uint16_t const RX_BUFFER_SIZE)
    : RX_BUFFER_SIZE(RX_BUFFER_SIZE)
    , _descriptors(descriptors)
    , _pbufAtIndex(pbufAtIndex)
    , _customPbufs(customPbufs)
    {
        for (size_t i = 0U; i < _descriptors.size(); i++)
        {
            _descriptors[i].data = &(rxBuffers[i * RX_BUFFER_SIZE]);
        }
    }

    void init();
    void interrupt();
    pbuf* readFrame(netif*& pNetif);

    uint32_t descriptorAddress() const { return reinterpret_cast<uint32_t>(_descriptors.data()); }

    pbuf* getCurrentBuffer();
    void handleBuffer(pbuf*& pFrame, pbuf* pCurrentBuffer) const;
    void handleLastBuffer(pbuf*& pFrame, pbuf*& pCompleteFrame, pbuf* pCurrentBuffer) const;

    bool _enableVlanTagging;

    ::lwiputils::PbufQueue _queue;

    ::etl::span<ENET_ERXD> const _descriptors;
    ::etl::span<pbuf*> const _pbufAtIndex;
    ::etl::span<::lwiputils::RxCustomPbuf> _customPbufs;

    uint8_t _nextFree = 0;
    uint8_t _nextBusy = 0;
};

} // namespace ethernet
