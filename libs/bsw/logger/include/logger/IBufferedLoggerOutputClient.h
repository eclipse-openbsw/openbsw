// Copyright 2024 Accenture.

#ifndef GUARD_A7D84007_32A4_4AE8_B310_36D2C8514C2B
#define GUARD_A7D84007_32A4_4AE8_B310_36D2C8514C2B

#include "logger/IEntryOutput.h"

#include <estd/uncopyable.h>

namespace logger
{
class ILoggerListener;

template<class E = uint32_t, class Timestamp = uint32_t>
class IBufferedLoggerOutputClient : private ::estd::uncopyable
{
public:
    IBufferedLoggerOutputClient();

    virtual void addListener(ILoggerListener& listener)          = 0;
    virtual void removeListener(ILoggerListener& listener)       = 0;
    virtual bool outputEntry(IEntryOutput<E, Timestamp>& output) = 0;
};

template<class E, class Timestamp>
inline IBufferedLoggerOutputClient<E, Timestamp>::IBufferedLoggerOutputClient()
: ::estd::uncopyable()
{}

} // namespace logger

#endif // GUARD_A7D84007_32A4_4AE8_B310_36D2C8514C2B
