// Copyright 2024 Accenture.

#include "util/logger/Logger.h"

#include "util/logger/ILoggerOutput.h"

#include <gtest/gtest.h>

using namespace ::util::logger;
using namespace ::util::format;

namespace
{
// NOLINTNEXTLINE(cert-dcl50-cpp): va_list usage only for printing functionalities.
void callLog(uint8_t index, Level level, char const* formatString, ...)
{
    va_list ap;
    va_start(ap, formatString);
    Logger::log(index, level, formatString, ap);
    va_end(ap);
}

struct LoggerTest
: public ::testing::Test
, public IComponentMapping
, public ILoggerOutput
{
    LoggerTest() : _enabled(false), _level(LEVEL_NONE), _componentIndex(COMPONENT_NONE) {}

    ~LoggerTest() override { Logger::shutdown(); }

    bool isEnabled(uint8_t componentIndex, Level level) const override
    {
        _componentIndex = componentIndex;
        _level          = level;
        return _enabled;
    }

    Level getLevel(uint8_t componentIndex) const override
    {
        _componentIndex = componentIndex;
        return _level;
    }

    LevelInfo getLevelInfo(Level level) const override
    {
        _level = level;
        return _levelInfo;
    }

    ComponentInfo getComponentInfo(uint8_t componentIndex) const override
    {
        _componentIndex = componentIndex;
        return _componentInfo;
    }

    void logOutput(
        ComponentInfo const& componentInfo,
        LevelInfo const& levelInfo,
        char const* str,
        va_list ap) override
    {
        _outComponentInfo = componentInfo;
        _outLevelInfo     = levelInfo;
        char buffer[300];
        static_cast<void>(vsnprintf(buffer, sizeof(buffer), str, ap));
        _logStr = buffer;
    }

    bool checkAndResetLog(
        uint8_t componentIndex,
        Level level,
        uint8_t outComponentIndex,
        Level outLevel,
        char const* str)
    {
        bool result = _componentIndex == componentIndex && _level == level
                      && (outComponentIndex == _outComponentInfo.getIndex())
                      && (outLevel == _outLevelInfo.getLevel()) && (_logStr == str);

        _componentIndex   = COMPONENT_NONE;
        _level            = LEVEL_NONE;
        _outComponentInfo = ComponentInfo();
        _outLevelInfo     = LevelInfo();
        _logStr.clear();
        return result;
    }

    bool _enabled;
    mutable Level _level;
    std::string _levelStr;
    LevelInfo _levelInfo;
    mutable uint8_t _componentIndex;
    ComponentInfo _componentInfo;
    std::string _logStr;
    LevelInfo _outLevelInfo;
    ComponentInfo _outComponentInfo;
};
} // anonymous namespace

TEST_F(LoggerTest, testEnabled)
{
    Logger::init(*this, *this);

    _enabled = true;
    ASSERT_TRUE(Logger::isEnabled(15, LEVEL_DEBUG));
    ASSERT_EQ(15, _componentIndex);
    ASSERT_EQ(LEVEL_DEBUG, _level);

    _enabled = false;
    ASSERT_FALSE(Logger::isEnabled(16, LEVEL_INFO));
    ASSERT_EQ(16, _componentIndex);
    ASSERT_EQ(LEVEL_INFO, _level);

    _enabled = true;
    ASSERT_TRUE(Logger::isEnabled(17, LEVEL_ERROR));
    ASSERT_EQ(17, _componentIndex);
    ASSERT_EQ(LEVEL_ERROR, _level);

    Logger::shutdown();
    ASSERT_FALSE(Logger::isEnabled(18, LEVEL_INFO));
    ASSERT_EQ(17, _componentIndex);
    ASSERT_EQ(LEVEL_ERROR, _level);
}

TEST_F(LoggerTest, testGetLevel)
{
    Logger::init(*this, *this);

    _level = LEVEL_DEBUG;
    ASSERT_EQ(LEVEL_DEBUG, Logger::getLevel(10));
    ASSERT_EQ(10, _componentIndex);

    _level = LEVEL_NONE;
    ASSERT_EQ(LEVEL_NONE, Logger::getLevel(11));
    ASSERT_EQ(11, _componentIndex);
}

TEST_F(LoggerTest, testLogging)
{
    Logger::init(*this, *this);

    ComponentInfo::PlainInfo constComponentInfo
        = {{"abc", {Color::DEFAULT_COLOR, 0U, Color::DEFAULT_COLOR}}};
    ComponentInfo componentInfo(12, &constComponentInfo);

    _enabled       = true;
    _componentInfo = componentInfo;
    _levelInfo     = LevelInfo(LevelInfo::getDefaultTable() + LEVEL_DEBUG);

    Logger::log(1, LEVEL_INFO, "abc: %d %s", 12, "log");
    ASSERT_TRUE(checkAndResetLog(1, LEVEL_INFO, 12, LEVEL_DEBUG, "abc: 12 log"));

    Logger::debug(2, "abc: %d %s", 13, "debug");
    ASSERT_TRUE(checkAndResetLog(2, LEVEL_DEBUG, 12, LEVEL_DEBUG, "abc: 13 debug"));

    Logger::info(3, "abc: %d %s", 14, "info");
    ASSERT_TRUE(checkAndResetLog(3, LEVEL_INFO, 12, LEVEL_DEBUG, "abc: 14 info"));

    Logger::warn(4, "abc: %d %s", 14, "warn");
    ASSERT_TRUE(checkAndResetLog(4, LEVEL_WARN, 12, LEVEL_DEBUG, "abc: 14 warn"));

    Logger::error(5, "abc: %d %s", 15, "error");
    ASSERT_TRUE(checkAndResetLog(5, LEVEL_ERROR, 12, LEVEL_DEBUG, "abc: 15 error"));

    Logger::critical(6, "abc: %d %s", 16, "critical");
    ASSERT_TRUE(checkAndResetLog(6, LEVEL_CRITICAL, 12, LEVEL_DEBUG, "abc: 16 critical"));

    callLog(7, LEVEL_INFO, "abc: %d %s", 17, "vlog");
    ASSERT_TRUE(checkAndResetLog(7, LEVEL_INFO, 12, LEVEL_DEBUG, "abc: 17 vlog"));
}

TEST_F(LoggerTest, testUninitializedUsage)
{
    ASSERT_FALSE(Logger::isEnabled(1, LEVEL_INFO));
    Logger::log(0, LEVEL_DEBUG, "abc", 1, 2, 3);
    Logger::info(1, "abc", 1, 2, 3);
    Logger::debug(2, "abc", 1, 2, 3);
    Logger::warn(3, "abc", 1, 2, 3);
    Logger::error(4, "abc", 1, 2, 3);
    Logger::critical(5, "abc", 1, 2, 3);
    callLog(6, LEVEL_DEBUG, "abc", 1, 2, 3);
    ASSERT_EQ(LEVEL_NONE, Logger::getLevel(0));
}
