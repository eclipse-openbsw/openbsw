// Copyright 2024 Accenture.

#pragma once

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
