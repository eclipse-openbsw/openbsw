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

#include "doip/common/DoIpStaticPayloadSendJob.h"

namespace doip
{
/**
 * A simple DoIP client vehicle announcement receiver service.
 * The service broadcasts/multicasts an initial vehicle identification and
 * listens for UDP announcement messages of connected devices.
 */
class DoIpVehicleIdentificationRequestSendJob : public DoIpStaticPayloadSendJob
{
public:
    /**
     * Constructor.
     * \param releaseCallback function that will be called when the send job will be released
     */
    explicit DoIpVehicleIdentificationRequestSendJob(ReleaseCallbackType releaseCallback);
};

} // namespace doip
