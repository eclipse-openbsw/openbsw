// Copyright 2024 Accenture.

#pragma once

#include "uds/base/Service.h"

namespace uds
{
class TesterPresent : public Service
{
public:
    TesterPresent();

private:
    static uint8_t const TESTER_PRESENT_ANSWER   = 0x00U;
    static uint8_t const EXPECTED_REQUEST_LENGTH = 1U;
    static uint8_t const RESPONSE_LENGTH         = 1U;

    DiagReturnCode::Type process(
        IncomingDiagConnection& connection,
        uint8_t const request[],
        uint16_t requestLength) override;
};

} // namespace uds

