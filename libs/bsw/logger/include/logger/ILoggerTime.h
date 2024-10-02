// Copyright 2024 Accenture.

#ifndef GUARD_2B64B466_7D26_4054_BFAC_3176DAC1D207
#define GUARD_2B64B466_7D26_4054_BFAC_3176DAC1D207

#include <util/stream/IOutputStream.h>

#include <estd/uncopyable.h>
#include <platform/estdint.h>

namespace logger
{
template<class T = uint32_t>
class ILoggerTime : private ::estd::uncopyable
{
public:
    using TimestampType = T;

    ILoggerTime();

    virtual T getTimestamp() const = 0;
    virtual void
    formatTimestamp(::util::stream::IOutputStream& outputStream, T const& timestamp) const
        = 0;
};

template<class T>
inline ILoggerTime<T>::ILoggerTime() : ::estd::uncopyable()
{}

} // namespace logger

#endif // GUARD_2B64B466_7D26_4054_BFAC_3176DAC1D207
