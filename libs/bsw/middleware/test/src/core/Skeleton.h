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
#include "middleware/core/SkeletonBase.h"
#include "middleware/core/types.h"

namespace middleware::core::test
{

class Skeleton : public ::middleware::core::SkeletonBase
{
public:
    Skeleton(uint16_t serviceId, uint16_t instanceId)
    : middleware::core::SkeletonBase(), _serviceId(serviceId)
    {
        this->setInstanceId(instanceId);
    }

    uint16_t getServiceId() const final { return _serviceId; }

    ::middleware::core::HRESULT onNewMessageReceived(::middleware::core::Message const&) override
    {
        return ::middleware::core::HRESULT::NotImplemented;
    }

private:
    uint16_t _serviceId;
};

} // namespace middleware::core::test
