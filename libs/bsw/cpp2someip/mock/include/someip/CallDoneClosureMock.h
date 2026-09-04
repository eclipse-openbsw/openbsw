/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "someip/RpcClosure.h"

#include <gmock/gmock.h>

namespace someip
{
class CallDoneClosureMock : public Closure<ServiceResultCode, ServiceResultCode>
{
public:
    MOCK_METHOD(ServiceResultCode, invoke, (ServiceResultCode param));

    ServiceResultCode operator()(ServiceResultCode param) override { return invoke(param); }
};

} // namespace someip
