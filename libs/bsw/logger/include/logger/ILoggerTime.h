// Copyright 2024 Accenture.

#pragma once

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

