// Copyright 2024 Accenture.

#include "logger/logger.h"

/* start: adding logger includes */

#include <lifecycle/LifecycleLogger.h>
#include <logger/ConsoleLogger.h>
#include <safeLifecycle/SafetyLogger.h>
#ifdef PLATFORM_SUPPORT_CAN
#include <can/CanLogger.h>
#include <docan/common/DoCanLogger.h>
#endif // PLATFORM_SUPPORT_CAN
#ifdef PLATFORM_SUPPORT_UDS
#include <transport/TpRouterLogger.h>
#include <uds/UdsLogger.h>
#endif // PLATFORM_SUPPORT_UDS
/* end: adding logger includes */

DEFINE_LOGGER_COMPONENT(BSP);
DEFINE_LOGGER_COMPONENT(COMMON);
DEFINE_LOGGER_COMPONENT(DEMO);
DEFINE_LOGGER_COMPONENT(GLOBAL);
DEFINE_LOGGER_COMPONENT(SAFETY);
DEFINE_LOGGER_COMPONENT(UDS);

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
LOGGER_COMPONENT_MAPPING_INFO(_DEBUG, DEMO, ::util::format::Color::DEFAULT_COLOR)
LOGGER_COMPONENT_MAPPING_INFO(_DEBUG, GLOBAL, ::util::format::Color::DEFAULT_COLOR)
LOGGER_COMPONENT_MAPPING_INFO(_DEBUG, LIFECYCLE, ::util::format::Color::DARK_GRAY)
LOGGER_COMPONENT_MAPPING_INFO(_DEBUG, CONSOLE, ::util::format::Color::DEFAULT_COLOR)
LOGGER_COMPONENT_MAPPING_INFO(_DEBUG, SAFETY, ::util::format::Color::DEFAULT_COLOR)
#ifdef PLATFORM_SUPPORT_CAN
LOGGER_COMPONENT_MAPPING_INFO(_DEBUG, CAN, ::util::format::Color::LIGHT_BLUE)
LOGGER_COMPONENT_MAPPING_INFO(_DEBUG, DOCAN, ::util::format::Color::LIGHT_GRAY)
#endif // PLATFORM_SUPPORT_CAN
#ifdef PLATFORM_SUPPORT_UDS
LOGGER_COMPONENT_MAPPING_INFO(_DEBUG, UDS, ::util::format::Color::LIGHT_YELLOW)
LOGGER_COMPONENT_MAPPING_INFO(_DEBUG, TPROUTER, ::util::format::Color::LIGHT_YELLOW)
#endif // PLATFORM_SUPPORT_UDS
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
LoggerComposition loggerComposition(loggerComponentMapping, "RefApp");

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

} // namespace logger
