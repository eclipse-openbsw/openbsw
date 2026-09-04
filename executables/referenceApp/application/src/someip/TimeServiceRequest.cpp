/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/TimeServiceRequest.h"

void TimeServiceRequest::serializeToArray(someip::SomeIpSerializer& /*serializer*/) const
{
    // do nothing
}

void TimeServiceRequest::parseFromArray(someip::SomeIpParser& /*parser*/)
{
    // do nothing
}

uint32_t TimeServiceRequest::getSize() const { return 0U; }
