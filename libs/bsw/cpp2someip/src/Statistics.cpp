/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/Statistics.h"

namespace someip
{
uint32_t Statistics::numIncomingSubscriptions;
uint32_t Statistics::maxNumIncomingSubscriptions;
uint32_t Statistics::numRemoteServices;
uint32_t Statistics::maxNumRemoteServices;
uint32_t Statistics::numProvidedServices;
uint32_t Statistics::maxNumProvidedServices;
uint32_t Statistics::numQueries;
uint32_t Statistics::maxNumQueries;

uint32_t Statistics::counters[static_cast<uint16_t>(Statistics::Counter::COUNTER_ARRAY_SIZE)];

// static
void Statistics::reset()
{
    for (auto& counter : counters)
    {
        counter = 0U;
    }
}

} // namespace someip
