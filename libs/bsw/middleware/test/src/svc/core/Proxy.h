/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "gtest/gtest.h"
#include "middleware/svc/core/ProxyBase.h"
#include "middleware/svc/core/types.h"

namespace middleware::svc::core::test
{

class Proxy : public ::middleware::svc::core::ProxyBase
{
public:
    Proxy(
        uint16_t serviceId,
        uint16_t instanceId,
        uint8_t addressId = ::etl::numeric_limits<uint8_t>::max())
    : ::middleware::svc::core::ProxyBase(), _serviceId(serviceId)
    {
        this->setAddressId(addressId);
        this->setInstanceId(instanceId);
    }

    uint16_t getServiceId() const final { return _serviceId; }

    virtual ::middleware::svc::core::HRESULT
    onNewMessageReceived(::middleware::svc::core::Message const&)
    {
        return ::middleware::svc::core::HRESULT::NotImplemented;
    }

private:
    uint16_t _serviceId;
};

} // namespace middleware::svc::core::test
