/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "util/format/IPrintfArgumentReader.h"
#include "util/format/Printf.h"

#include <util/estd/va_list_ref.h>

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
