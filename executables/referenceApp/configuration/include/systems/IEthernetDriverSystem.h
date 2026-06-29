/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

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
    virtual ::lwiputils::PbufQueue& getRx()                                               = 0;
};
} // namespace ethernet
