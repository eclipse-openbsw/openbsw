// Copyright 2024 Accenture.

#pragma once

#include <estd/slice.h>
#include <estd/uncopyable.h>
#include <platform/estdint.h>

namespace logger
{
class IPersistenceManager : public ::estd::uncopyable
{
public:
    virtual bool writeMapping(::estd::slice<uint8_t const> const& src) const            = 0;
    virtual ::estd::slice<uint8_t const> readMapping(::estd::slice<uint8_t> dest) const = 0;

protected:
    IPersistenceManager() {}
};

} // namespace logger

