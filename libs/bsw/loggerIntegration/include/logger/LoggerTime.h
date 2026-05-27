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

#include <logger/ILoggerTime.h>

namespace logger
{
class LoggerTime : public ILoggerTime<int64_t>
{
public:
    explicit LoggerTime(char const* timestampFormat);

    virtual int64_t getTimestamp() const override;
    virtual void
    formatTimestamp(::util::stream::IOutputStream& stream, int64_t const& timestamp) const override;

private:
    char const* const _timestampFormat;
};

} // namespace logger
