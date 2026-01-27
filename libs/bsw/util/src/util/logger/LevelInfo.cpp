// Copyright 2024 Accenture.

#include "util/logger/LevelInfo.h"

#include "util/format/AttributedString.h"
#ifndef LOGGER_NO_LEGACY_API

namespace util
{
namespace logger
{
using ::util::format::BOLD;

constexpr LevelInfo::PlainInfo LevelInfo::_defaultConstLevelInfos[LEVEL_COUNT]
    = {{{"DEBUG", {format::Color::DEFAULT_COLOR, 0U, format::Color::DEFAULT_COLOR}}, LEVEL_DEBUG},
       {{"INFO", {format::Color::DEFAULT_COLOR, 0U, format::Color::DEFAULT_COLOR}}, LEVEL_INFO},
       {{"WARN", {format::Color::YELLOW, BOLD, format::Color::DEFAULT_COLOR}}, LEVEL_WARN},
       {{"ERROR", {format::Color::RED, BOLD, format::Color::DEFAULT_COLOR}}, LEVEL_ERROR},
       {{"CRITICAL", {format::Color::DEFAULT_COLOR, 0U, format::Color::DEFAULT_COLOR}},
        LEVEL_CRITICAL},
       {{"NONE", {format::Color::DEFAULT_COLOR, 0U, format::Color::DEFAULT_COLOR}}, LEVEL_NONE}};

LevelInfo::TableType LevelInfo::getDefaultTable() { return _defaultConstLevelInfos; }

LevelInfo& LevelInfo::operator=(LevelInfo const& src)
{
    if (this != &src)
    {
        _plainInfo = src._plainInfo;
    }
    return *this;
}

} /* namespace logger */
} /* namespace util */

#endif /* LOGGER_NO_LEGACY_API */
