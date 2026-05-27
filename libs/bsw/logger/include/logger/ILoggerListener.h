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

#include <etl/intrusive_list.h>
#include <etl/uncopyable.h>

namespace logger
{
class ILoggerListener
: public ::etl::bidirectional_link<0>
, private ::etl::uncopyable
{
public:
    ILoggerListener();

    virtual void logAvailable() = 0;
};

inline ILoggerListener::ILoggerListener() : ::etl::bidirectional_link<0>(), ::etl::uncopyable() {}

} // namespace logger
