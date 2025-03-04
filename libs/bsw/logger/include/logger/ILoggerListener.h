// Copyright 2024 Accenture.

#ifndef GUARD_88C26625_9E67_4D69_A325_283FE20FA7BE
#define GUARD_88C26625_9E67_4D69_A325_283FE20FA7BE

#include <etl/intrusive_forward_list.h>
#include <etl/uncopyable.h>

namespace logger
{
class ILoggerListener
: public ::etl::forward_link<0>
, private ::etl::uncopyable
{
public:
    ILoggerListener();

    virtual void logAvailable() = 0;
};

inline ILoggerListener::ILoggerListener() : ::etl::forward_link<0>(), ::etl::uncopyable() {}

} // namespace logger

#endif // GUARD_88C26625_9E67_4D69_A325_283FE20FA7BE
