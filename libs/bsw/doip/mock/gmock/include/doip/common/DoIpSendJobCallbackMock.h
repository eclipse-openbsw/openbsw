/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

/**
 * \ingroup doip
 */
#pragma once

#include "doip/common/IDoIpSendJobCallback.h"

#include <gmock/gmock.h>

namespace doip
{
/**
 * Mock for DoIP send job callback.
 * \tparam T send job class type
 */
template<class T>
class DoIpSendJobCallbackMock : public IDoIpSendJobCallback<T>
{
public:
    MOCK_METHOD2_T(releaseSendJob, void(T& sendJob, bool success));
};

} // namespace doip
