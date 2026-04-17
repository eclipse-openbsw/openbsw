// Copyright 2025 Accenture.

#include "logger/logger.h"

/* start: adding logger includes */

#include <lifecycle/LifecycleLogger.h>
#include <logger/ConsoleLogger.h>
#include <printf/printf.h>

/* end: adding logger includes */

DEFINE_LOGGER_COMPONENT(BSP);
DEFINE_LOGGER_COMPONENT(COMMON);
DEFINE_LOGGER_COMPONENT(GLOBAL);

#include <async/AsyncBinding.h>
#include <console/AsyncCommandWrapper.h>
#include <logger/ComponentConfig.h>
#include <logger/ComponentMapping.h>
#include <logger/DefaultLoggerCommand.h>
#include <logger/LoggerComposition.h>

START_LOGGER_COMPONENT_MAPPING_INFO_TABLE(loggerComponentInfoTable)
/* start: adding logger components */
LOGGER_COMPONENT_MAPPING_INFO(_DEBUG, BSP, ::util::format::Color::DEFAULT_COLOR)
LOGGER_COMPONENT_MAPPING_INFO(_DEBUG, COMMON, ::util::format::Color::DEFAULT_COLOR)
LOGGER_COMPONENT_MAPPING_INFO(_DEBUG, GLOBAL, ::util::format::Color::DEFAULT_COLOR)
LOGGER_COMPONENT_MAPPING_INFO(_DEBUG, LIFECYCLE, ::util::format::Color::DARK_GRAY)
LOGGER_COMPONENT_MAPPING_INFO(_DEBUG, CONSOLE, ::util::format::Color::DEFAULT_COLOR)
/* end: adding logger components */
END_LOGGER_COMPONENT_MAPPING_INFO_TABLE();

DEFINE_LOGGER_COMPONENT_MAPPING(
    LoggerComponentMappingType,
    loggerComponentMapping,
    loggerComponentInfoTable,
    ::util::logger::LevelInfo::getDefaultTable(),
    GLOBAL);

namespace logger
{
using ComponentConfigType = ComponentConfig<LoggerComponentMappingType::MappingSize>;
ComponentConfigType loggerComponentConfig(loggerComponentMapping);
LoggerComposition loggerComposition(loggerComponentMapping, "integrationTest");

DefaultLoggerCommand loggerCommand(loggerComponentConfig);
::console::AsyncCommandWrapper
    asyncCommandWrapper(loggerCommand.root(), ::async::AsyncBinding::AdapterType::TASK_IDLE);

void init()
{
    loggerComposition.start(
        LoggerComposition::ConfigStart::create<ComponentConfigType, &ComponentConfigType::start>(
            loggerComponentConfig));
}

void run() { loggerComposition.run(); }

void flush()
{
    loggerComposition.stop(
        LoggerComposition::ConfigStop::create<ComponentConfigType, &ComponentConfigType::shutdown>(
            loggerComponentConfig));
}

using ::util::logger::Logger;

extern "C" void log_lwipInfo(char const* message, ...)
{
    // to print variable argument list
    // ToDo: use StringWriter or PrintfFormatter available in util

    static char buffer[100];
    va_list ap;
    /* get the varargs */
    va_start(ap, message);
    vsnprintf_(buffer, sizeof(buffer), message, ap);
    va_end(ap);
}

} // namespace logger
