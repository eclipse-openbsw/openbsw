// Copyright 2024 Accenture.

#pragma once

#include "util/logger/LoggerBinding.h"

#ifndef LOGGER_NO_LEGACY_API

#include "util/format/AttributedString.h"

#include <etl/error_handler.h>

// Currently we still define the enum value _DEBUG to make the levels compliant to the former Logger
// levels. Unfortunately on Visual Studio this is a predefined macro which we will undefine here
// until we can remove support of the deprecated Logger levels starting with underscore.
#ifdef _DEBUG
#undef _DEBUG
#endif // _DEBUG

namespace util
{
namespace logger
{
/**
 * Enumeration defining levels/severities for log messages. The value NONE is only used for allowing
 * to filter out all messages.
 */
enum Level
{
    LEVEL_DEBUG    = 0,
    LEVEL_INFO     = 1,
    LEVEL_WARN     = 2,
    LEVEL_ERROR    = 3,
    LEVEL_CRITICAL = 4,
    LEVEL_NONE     = 5,

    _DEBUG     = LEVEL_DEBUG,
    _INFO      = LEVEL_INFO,
    _WARN      = LEVEL_WARN,
    _ERROR     = LEVEL_ERROR,
    _CRITICAL  = LEVEL_CRITICAL,
    _NOLOGGING = LEVEL_NONE
};

uint8_t const LEVEL_COUNT = 6U;

/**
 * class encapsulating logger level info.
 */
class LevelInfo
{
public:
    /**
     * structure that allows placing the information in constant memory.
     */
    struct PlainInfo
    {
        ::util::format::PlainAttributedString _nameInfo;
        Level _level;
    };

    using TableType = PlainInfo const (&)[6];

    /**
     * Static function that returns a default severity info table.
     * \return Reference to constant level infos
     */
    static TableType getDefaultTable();

    /**
     * Constructor
     * \param plainInfo pointer to a constant info structure. If 0 the info object will be invalid.
     */
    explicit LevelInfo(PlainInfo const* const plainInfo = nullptr) : _plainInfo(plainInfo) {}

    LevelInfo(LevelInfo const& src) = default;

    LevelInfo& operator=(LevelInfo const& src);

    /**
     * Check for validity of info structure.
     * \return true if initialized
     */
    bool isValid() const { return _plainInfo != nullptr; }

    /**
     * Get the name of the level. Should only be called if isValid() returns true.
     * \return attributed name of the level.
     */
    ::util::format::AttributedString getName() const
    {
        ETL_ASSERT(_plainInfo != nullptr, ETL_ERROR_GENERIC("info must not be null"));
        return ::util::format::AttributedString(_plainInfo->_nameInfo);
    }

    /**
     * \return the level value.
     */
    Level getLevel() const
    {
        ETL_ASSERT(_plainInfo != nullptr, ETL_ERROR_GENERIC("info must not be null"));
        return _plainInfo->_level;
    }

    /**
     * Get the zero-terminated string. Should only be called if isValid() returns true.
     * \return the zero-terminated string
     */
    char const* getPlainInfoString() const
    {
        ETL_ASSERT(_plainInfo != nullptr, ETL_ERROR_GENERIC("info must not be null"));
        return _plainInfo->_nameInfo._string;
    }

private:
    PlainInfo const* _plainInfo;
    static PlainInfo const _defaultConstLevelInfos[LEVEL_COUNT];
};

} // namespace logger
} // namespace util

#endif // LOGGER_NO_LEGACY_API
