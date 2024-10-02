// Copyright 2024 Accenture.

#ifndef GUARD_9471276B_9E06_4191_A774_FCC7B3092B77
#define GUARD_9471276B_9E06_4191_A774_FCC7B3092B77

#include "uds/base/Service.h"

namespace uds
{
class ControlDTCSetting : public Service
{
public:
    enum DtcMode
    {
        CONTROL_DTC_SETTING_ON  = 0x01,
        CONTROL_DTC_SETTING_OFF = 0x02
    };

    explicit ControlDTCSetting();

private:
    static uint8_t const EXPECTED_REQUEST_LENGTH = 1U;
    static uint8_t const RESPONSE_LENGTH         = 1U;

    DiagReturnCode::Type process(
        IncomingDiagConnection& connection,
        uint8_t const request[],
        uint16_t requestLength) override;
};

} // namespace uds

#endif // GUARD_9471276B_9E06_4191_A774_FCC7B3092B77
