/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "uds/base/Service.h"

namespace uds
{
class DiagMgr;

/**
 * UDS service ControlDTCSetting (0x85).
 * When DiagMgr is wired, ON/OFF state is forwarded to FaultManager
 * which gates all DTC confirmations
 */
class ControlDTCSetting : public Service
{
public:
    enum DtcMode
    {
        CONTROL_DTC_SETTING_ON  = 0x01,
        CONTROL_DTC_SETTING_OFF = 0x02
    };

    explicit ControlDTCSetting();

    /** Wire DiagMgr backend. Call during UdsSystem::init(). */
    void setDiagMgr(DiagMgr* diagMgr) { _diagMgr = diagMgr; }

private:
    static uint8_t const EXPECTED_REQUEST_LENGTH = 1U;
    static uint8_t const RESPONSE_LENGTH         = 1U;

    DiagReturnCode::Type process(
        IncomingDiagConnection& connection,
        uint8_t const request[],
        uint16_t requestLength) override;

    DiagMgr* _diagMgr;
};

} // namespace uds
