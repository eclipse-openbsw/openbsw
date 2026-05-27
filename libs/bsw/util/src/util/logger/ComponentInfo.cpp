/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "util/logger/ComponentInfo.h"

#ifndef LOGGER_NO_LEGACY_API

#include <etl/error_handler.h>

namespace util
{
namespace logger
{
ComponentInfo& ComponentInfo::operator=(ComponentInfo const& src)
{
    if (this != &src)
    {
        _componentIndex = src._componentIndex;
        _plainInfo      = src._plainInfo;
    }
    return *this;
}

::util::format::AttributedString ComponentInfo::getName() const
{
    ETL_ASSERT(_plainInfo != nullptr, ETL_ERROR_GENERIC("info must not be null"));
    return ::util::format::AttributedString(_plainInfo->_nameInfo);
}

} /* namespace logger */
} /* namespace util */

#endif /* LOGGER_NO_LEGACY_API */
