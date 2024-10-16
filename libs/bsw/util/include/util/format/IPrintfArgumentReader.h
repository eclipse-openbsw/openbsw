// Copyright 2024 Accenture.

#ifndef GUARD_4207236E_CB3F_4B0B_8D09_3C809E11D82A
#define GUARD_4207236E_CB3F_4B0B_8D09_3C809E11D82A

#include "util/format/Printf.h"

namespace util
{
namespace format
{
/**
 * This interface is the base interface for providing data to a
 * ::util::format::PrintfFormatter.
 */
class IPrintfArgumentReader
{
public:
    IPrintfArgumentReader() = default;

    IPrintfArgumentReader(IPrintfArgumentReader const&)            = delete;
    IPrintfArgumentReader& operator=(IPrintfArgumentReader const&) = delete;

    /**
     * Get the next argument value. The expected datatype is provided.
     * \param datatype expected datatype of the argument.
     * \return Pointer the the parameter value if existing. The provided value
     * is only valid up to the next call to readArgument, 0 otherwise
     */
    virtual ParamVariant const* readArgument(ParamDatatype datatype) = 0;
};

} // namespace format
} // namespace util

#endif /* GUARD_4207236E_CB3F_4B0B_8D09_3C809E11D82A */
