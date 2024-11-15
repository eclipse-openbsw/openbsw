// Copyright 2024 Accenture.

#pragma once

#include <etl/intrusive_list.h>
#include <etl/uncopyable.h>

namespace logger
{
class ILoggerListener
: public ::etl::bidirectional_link<0>
, private ::etl::uncopyable
{
public:
    ILoggerListener();

    virtual void logAvailable() = 0;
};

inline ILoggerListener::ILoggerListener() : ::etl::bidirectional_link<0>(), ::etl::uncopyable() {}

} // namespace logger
