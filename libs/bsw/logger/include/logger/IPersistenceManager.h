// Copyright 2024 Accenture.

#ifndef GUARD_74944026_98DF_43E9_8445_8984DCF45DAA
#define GUARD_74944026_98DF_43E9_8445_8984DCF45DAA

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

#endif // GUARD_74944026_98DF_43E9_8445_8984DCF45DAA
