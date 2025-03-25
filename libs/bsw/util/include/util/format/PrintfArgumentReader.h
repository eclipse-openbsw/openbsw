// Copyright 2024 Accenture.

#pragma once

#include "util/format/IPrintfArgumentReader.h"
#include "util/format/Printf.h"

#include <estd/va_list_ref.h>

namespace util
{
namespace format
{
/**
 * Class that encapsulates a C style list of arguments (va_list type).
 */
class PrintfArgumentReader : public IPrintfArgumentReader
{
public:
    explicit PrintfArgumentReader(::estd::va_list_ref ap);

    ParamVariant const* readArgument(ParamDatatype datatype) override;

private:
    ::estd::va_list_ref _ap;
    ParamVariant _variant;
};
} // namespace format
} // namespace util

