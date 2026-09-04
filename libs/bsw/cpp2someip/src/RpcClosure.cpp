/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/RpcClosure.h"

namespace someip
{

// virtual
ServiceResultCode CallDoneSyncClosure::operator()(ServiceResultCode param)
{
    _result = param;
    return ServiceResultCode::RPC_SENT_SUCCESSFULLY;
}

} // namespace someip
