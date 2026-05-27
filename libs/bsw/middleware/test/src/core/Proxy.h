/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "gtest/gtest.h"
#include "middleware/core/ProxyBase.h"
#include "middleware/core/types.h"

namespace middleware::core::test
{

class Proxy : public ::middleware::core::ProxyBase
{
public:
    Proxy(
        uint16_t serviceId,
        uint16_t instanceId,
        uint16_t addressId = etl::numeric_limits<uint16_t>::max())
    : ::middleware::core::ProxyBase(), _serviceId(serviceId)
    {
        this->setAddressId(addressId);
        this->setInstanceId(instanceId);
    }

    uint16_t getServiceId() const final { return _serviceId; }

    virtual ::middleware::core::HRESULT onNewMessageReceived(::middleware::core::Message const&)
    {
        return ::middleware::core::HRESULT::NotImplemented;
    }

private:
    uint16_t _serviceId;
};

} // namespace middleware::core::test
