// Copyright 2024 Accenture.

#include "util/logger/ComponentInfo.h"

#ifndef LOGGER_NO_LEGACY_API

#include <util/estd/assert.h>

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
    estd_assert(_plainInfo != nullptr);
    return ::util::format::AttributedString(_plainInfo->_nameInfo);
}

} /* namespace logger */
} /* namespace util */

#endif /* LOGGER_NO_LEGACY_API */
