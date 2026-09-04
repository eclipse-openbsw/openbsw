/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/ProvidedServiceData.h"

#include "someip/SomeIpParser.h"
#include "someip/SomeIpSerializer.h"
#include "someip/defaultlogger.h"
#include "someip/logger.h"

using ::util::logger::SOMEIP;

void ProvidedServiceData::serializeToArray(someip::SomeIpSerializer& serializer) const
{
    DEBUG_LOG(SOMEIP, "ProvidedServiceData::serializeToArray()");
    serializer << data;
}

void ProvidedServiceData::parseFromArray(someip::SomeIpParser& parser)
{
    DEBUG_LOG(SOMEIP, "ProvidedServiceData::parseFromArray()");
    parser >> data;
}

uint32_t ProvidedServiceData::getSize() const
{
    DEBUG_LOG(SOMEIP, "ProvidedServiceData::getSize()");
    return sizeof(data);
}
