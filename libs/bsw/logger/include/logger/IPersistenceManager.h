// Copyright 2024 Accenture.

#pragma once

#include <etl/span.h>
#include <etl/uncopyable.h>

#include <platform/estdint.h>

namespace logger
{
class IPersistenceManager : public ::etl::uncopyable
{
public:
    virtual bool writeMapping(::etl::span<uint8_t const> const& src) const          = 0;
    virtual ::etl::span<uint8_t const> readMapping(::etl::span<uint8_t> dest) const = 0;

protected:
    IPersistenceManager() {}
};

} // namespace logger
