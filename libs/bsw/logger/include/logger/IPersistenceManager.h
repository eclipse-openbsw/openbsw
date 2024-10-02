// Copyright 2024 Accenture.

#ifndef GUARD_74944026_98DF_43E9_8445_8984DCF45DAA
#define GUARD_74944026_98DF_43E9_8445_8984DCF45DAA

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

#endif // GUARD_74944026_98DF_43E9_8445_8984DCF45DAA
