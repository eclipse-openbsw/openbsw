/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "doip/common/DoIpVehicleIdentificationRequestSendJob.h"

#include "doip/common/DoIpConstants.h"

namespace doip
{
DoIpVehicleIdentificationRequestSendJob::DoIpVehicleIdentificationRequestSendJob(
    ReleaseCallbackType const releaseCallback)
: DoIpStaticPayloadSendJob(
    0xffU,
    DoIpConstants::PayloadTypes::VEHICLE_IDENTIFICATION_REQUEST_MESSAGE,
    ::etl::span<uint8_t const>(),
    releaseCallback)
{}

} // namespace doip
