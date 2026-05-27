/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "uds/ReadIdentifierPot.h"

#include <etl/unaligned_type.h>
#ifdef PLATFORM_SUPPORT_IO
#include "bsp/adc/AnalogInputScale.h"
#include "outputPwm/OutputPwm.h"
#endif

#include "uds/connection/IncomingDiagConnection.h"

namespace uds
{

#ifdef PLATFORM_SUPPORT_IO
using bios::AnalogInput;
using bios::AnalogInputScale;
using bios::OutputPwm;
#endif

ReadIdentifierPot::ReadIdentifierPot(DiagSessionMask const sessionMask)
: DataIdentifierJob(_implementedRequest, sessionMask)
{
    constexpr uint32_t identifier = 0xCF02;
    _implementedRequest[0]        = 0x22U;
    _implementedRequest[1]        = (identifier >> 8) & 0xFFU;
    _implementedRequest[2]        = identifier & 0xFFU;
}

DiagReturnCode::Type ReadIdentifierPot::process(
    IncomingDiagConnection& connection,
    uint8_t const* const /* request */,
    uint16_t const /* requestLength */)
{
    PositiveResponse& response = connection.releaseRequestGetResponse();

    uint32_t adcValue = 0x00000002;

#ifdef PLATFORM_SUPPORT_IO
    (void)AnalogInputScale::get(AnalogInput::AiEVAL_POTI_ADC, adcValue);
#endif

    ::etl::be_int32_t responseData22Cf02(adcValue);
    (void)response.appendData(
        static_cast<uint8_t*>(responseData22Cf02.data()), responseData22Cf02.size());
    (void)connection.sendPositiveResponseInternal(response.getLength(), *this);

    return DiagReturnCode::OK;
}

} // namespace uds
