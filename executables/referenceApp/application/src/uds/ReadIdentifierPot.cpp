// Copyright 2024 Accenture.

#include "uds/ReadIdentifierPot.h"

#include "estd/big_endian.h"
#ifdef PLATFORM_SUPPORT_IO
#include "bsp/adc/AnalogInputScale.h"
#include "outputPwm/OutputPwm.h"
#endif

#include "uds/UdsLogger.h"
#include "uds/connection/IncomingDiagConnection.h"

using ::estd::array;
using ::estd::slice;

namespace uds
{
using ::util::logger::Logger;
using ::util::logger::UDS;

#ifdef PLATFORM_SUPPORT_IO
using bios::AnalogInput;
using bios::AnalogInputScale;
using bios::OutputPwm;
#endif

using ::estd::slice;

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
    uint8_t responseData22Cf02[4];

#ifdef PLATFORM_SUPPORT_IO
    (void)AnalogInputScale::get(AnalogInput::AiEVAL_POTI_ADC, adcValue);
#endif

    ::estd::write_be(&responseData22Cf02[0], adcValue);

    ::estd::slice<uint8_t> _responseSlice(responseData22Cf02);

    (void)response.appendData(_responseSlice.data(), static_cast<uint16_t>(_responseSlice.size()));
    (void)connection.sendPositiveResponseInternal(response.getLength(), *this);

    return DiagReturnCode::OK;
}

} // namespace uds
