// Copyright 2024 Accenture.

#ifndef GUARD_88C26625_9E67_4D69_A325_283FE20FA7BE
#define GUARD_88C26625_9E67_4D69_A325_283FE20FA7BE

#include <estd/forward_list.h>
#include <estd/uncopyable.h>

namespace logger
{
class ILoggerListener
: public ::estd::forward_list_node<ILoggerListener>
, private ::estd::uncopyable
{
public:
    ILoggerListener();

    virtual void logAvailable() = 0;
};

inline ILoggerListener::ILoggerListener()
: ::estd::forward_list_node<ILoggerListener>(), ::estd::uncopyable()
{}

} // namespace logger

#endif // GUARD_88C26625_9E67_4D69_A325_283FE20FA7BE
