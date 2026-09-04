/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/TimeServiceReply.h"

#include "someip/EtlString.h"
#include "someip/SomeIpParser.h"
#include "someip/StringEncoding.h"

#include <etl/error_handler.h>

void TimeServiceReply::serializeToArray(someip::SomeIpSerializer& /*serializer*/) const
{
    // do nothing - it's a reply
}

void TimeServiceReply::parseFromArray(someip::SomeIpParser& parser)
{
    ETL_ASSERT(
        parser.bytesAvailable() == _timeStr.max_size(),
        ETL_ERROR_GENERIC("parser bytes available must equal time string max size"));
    someip::StaticStringParser::readString(
        parser, _timeStr, someip::Encoding::SOMEIP_ENCODING_NO_BOM_NO_ZERO);
}

uint32_t TimeServiceReply::getSize() const { return _timeStr.size(); }
