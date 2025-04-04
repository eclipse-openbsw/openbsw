// Copyright 2024 Accenture.

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
