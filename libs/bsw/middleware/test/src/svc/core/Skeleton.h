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
#include "middleware/svc/core/SkeletonBase.h"
#include "middleware/svc/core/types.h"

namespace middleware::svc::core::test
{

class Skeleton : public ::middleware::svc::core::SkeletonBase
{
public:
    Skeleton(uint16_t serviceId, uint16_t instanceId)
    : middleware::svc::core::SkeletonBase(), _serviceId(serviceId)
    {
        this->setInstanceId(instanceId);
    }

    uint16_t getServiceId() const final { return _serviceId; }

    ::middleware::svc::core::HRESULT
    onNewMessageReceived(::middleware::svc::core::Message const&) override
    {
        return ::middleware::svc::core::HRESULT::NotImplemented;
    }

private:
    uint16_t _serviceId;
};

} // namespace middleware::svc::core::test
