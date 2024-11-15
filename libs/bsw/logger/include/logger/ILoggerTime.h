// Copyright 2024 Accenture.

#pragma once

#include <etl/uncopyable.h>
#include <util/stream/IOutputStream.h>

#include <platform/estdint.h>

namespace logger
{
template<class T = uint32_t>
class ILoggerTime : private ::etl::uncopyable
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
inline ILoggerTime<T>::ILoggerTime() : ::etl::uncopyable()
{}

} // namespace logger
