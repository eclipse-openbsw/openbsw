/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/SomeIpStreamer.h"

namespace someip
{
void validate(SomeIpStreamer& streamer, bool const result)
{
    if (streamer.isGood() && (result == false))
    {
        streamer.setFailure(::someip::ErrorCode::SOMEIP_CODING);
    }
}

} // namespace someip
