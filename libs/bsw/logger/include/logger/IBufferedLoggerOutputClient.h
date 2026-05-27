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

#include "logger/IEntryOutput.h"

#include <etl/uncopyable.h>

namespace logger
{
class ILoggerListener;

template<class E = uint32_t, class Timestamp = uint32_t>
class IBufferedLoggerOutputClient : private ::etl::uncopyable
{
public:
    IBufferedLoggerOutputClient();

    virtual void addListener(ILoggerListener& listener)          = 0;
    virtual void removeListener(ILoggerListener& listener)       = 0;
    virtual bool outputEntry(IEntryOutput<E, Timestamp>& output) = 0;
};

template<class E, class Timestamp>
inline IBufferedLoggerOutputClient<E, Timestamp>::IBufferedLoggerOutputClient()
: ::etl::uncopyable()
{}

} // namespace logger
