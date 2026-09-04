/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/ProvidedServiceImpl.h"

#include "someip/SomeIpConstants.h"
#include "someip/defaultlogger.h"
#include "someip/logger.h"

#include <someip/IEventSender.h>

#include <etl/error_handler.h>
#include <etl/span.h>

using ::util::logger::SOMEIP;

void callMethod(
    uint32_t dummy,
    someip::IEventSender& eventSender,
    uint16_t servicePort,
    uint16_t methodId,
    ProvidedServiceData const* request,
    ProvidedServiceData* response,
    someip::CallDoneClosure& done)
{
    DEBUG_LOG(SOMEIP, "%s instance called, data - %d", __PRETTY_FUNCTION__, dummy);
    WARN_LOG(SOMEIP, "request data 0x%x", request->data);
    response->data = (request->data >> 16) | (request->data << 16);
    WARN_LOG(SOMEIP, "response is 0x%x", response->data);

    auto const tmp = uint32_t{0x8000 + dummy};
    ETL_ASSERT(tmp > 0x8000, ETL_ERROR_GENERIC("tmp value must be greater than 0x8000"));
    uint16_t const eventGroup[] = {static_cast<uint16_t>(tmp)};
    auto tmp_slice              = ::etl::span<uint16_t const>(eventGroup);

    if (methodId == 1)
    {
        eventSender.sendEvent(
            0xCAFE,
            1,
            1,
            eventGroup[0],
            tmp_slice,
            0,
            response,
            servicePort,
            uint8_t(::someip::proto::SD_L4_PROTO_UDP));
        // clang-format on
    }

    done(someip::RPC_POSITIVE_RESPONSE);
}
