// Copyright 2024 Accenture.

#ifndef GUARD_30080D7C_6B2C_46C6_B4F2_A09CD5AB0C1E
#define GUARD_30080D7C_6B2C_46C6_B4F2_A09CD5AB0C1E

#include "util/logger/IComponentMapping.h"
#include "util/logger/ILoggerOutput.h"

namespace util
{
namespace logger
{
class LoggerComponentInfo
{
public:
    LoggerComponentInfo(uint8_t& component, char const* name, Level level);

    uint8_t& getComponent();
    ComponentInfo getComponentInfo() const;
    Level getLevel() const;

private:
    uint8_t& _component;
    Level _level;
    ComponentInfo::PlainInfo _componentInfo;
};

class TestConsoleLogger
: private IComponentMapping
, private ILoggerOutput
{
public:
    template<uint8_t Count>
    TestConsoleLogger(LoggerComponentInfo (&firstComponentInfo)[Count]);
    ~TestConsoleLogger();

    static void init();
    static void shutdown();

    bool isEnabled(uint8_t componentIndex, Level level) const override;
    Level getLevel(uint8_t componentIndex) const override;
    LevelInfo getLevelInfo(Level level) const override;
    ComponentInfo getComponentInfo(uint8_t componentIndex) const override;

    void logOutput(
        ComponentInfo const& componentInfo,
        LevelInfo const& levelInfo,
        char const* str,
        va_list ap) override;

private:
    void applyMapping();
    void clearMapping();

    LoggerComponentInfo* _firstComponent;
    uint8_t _count;
    TestConsoleLogger* _prevInstance;

    static TestConsoleLogger* _instance;
};

class TestLoggingGuard
{
public:
    TestLoggingGuard();
    ~TestLoggingGuard();
};

template<uint8_t Count>
TestConsoleLogger::TestConsoleLogger(LoggerComponentInfo (&firstComponentInfo)[Count])
: _firstComponent(firstComponentInfo), _count(Count), _prevInstance(_instance)
{
    _instance = this;
}

} // namespace logger
} // namespace util

#endif // GUARD_30080D7C_6B2C_46C6_B4F2_A09CD5AB0C1E
