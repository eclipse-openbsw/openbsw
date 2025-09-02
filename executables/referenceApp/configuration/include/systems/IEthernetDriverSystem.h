// Copyright 2024 Accenture.

#pragma once

#include <lwipSocket/netif/LwipNetworkInterface.h>
#include <lwipSocket/utils/LwipHelper.h>

namespace ethernet
{

class IEthernetDriverSystem
{
protected:
    ~IEthernetDriverSystem() = default;

public:
    virtual void setGroupcastAddressRecognition(::etl::array<uint8_t, 6> const mac) const = 0;
    virtual bool getLinkStatus(size_t port)                                               = 0;
    virtual bool writeFrame(struct netif* const aNetif, struct pbuf* const buf)           = 0;
    virtual ::lwiputils::PbufQueue::Receiver getRx()                                      = 0;
};
} // namespace ethernet
