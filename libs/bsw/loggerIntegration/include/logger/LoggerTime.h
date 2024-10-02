// Copyright 2024 Accenture.

#ifndef GUARD_015291CF_8D37_4391_83A8_66A6C8944AEB
#define GUARD_015291CF_8D37_4391_83A8_66A6C8944AEB

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

#endif /* GUARD_015291CF_8D37_4391_83A8_66A6C8944AEB */
