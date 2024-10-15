// Copyright 2024 Accenture.

#include "logger/logger.h"

/* start: adding logger includes */

#include <lifecycle/LifecycleLogger.h>
#include <logger/ConsoleLogger.h>
#include <printf/printf.h>
#include <safeLifecycle/SafetyLogger.h>
#ifdef PLATFORM_SUPPORT_CAN
#include <can/CanLogger.h>
#include <docan/common/DoCanLogger.h>
#endif // PLATFORM_SUPPORT_CAN
#ifdef PLATFORM_SUPPORT_UDS
#include <transport/TpRouterLogger.h>
#include <uds/UdsLogger.h>
#endif // PLATFORM_SUPPORT_UDS
#ifdef PLATFORM_SUPPORT_ETHERNET
#include <ethernet/EthernetLogger.h>
#include <lwipSocket/utils/LwipLogger.h>
#include <tcp/TcpLogger.h>
#include <udp/UdpLogger.h>
#endif // PLATFORM_SUPPORT_ETHERNET

/* end: adding logger includes */

DEFINE_LOGGER_COMPONENT(BSP);
DEFINE_LOGGER_COMPONENT(COMMON);
DEFINE_LOGGER_COMPONENT(DEMO);
DEFINE_LOGGER_COMPONENT(GLOBAL);
DEFINE_LOGGER_COMPONENT(SAFETY);
DEFINE_LOGGER_COMPONENT(ETHERNET);
DEFINE_LOGGER_COMPONENT(UDS);
DEFINE_LOGGER_COMPONENT(LWIP);

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
#ifdef PLATFORM_SUPPORT_ETHERNET
LOGGER_COMPONENT_MAPPING_INFO(_DEBUG, ETHERNET, ::util::format::Color::MAGENTA)
LOGGER_COMPONENT_MAPPING_INFO(_DEBUG, TCP, ::util::format::Color::CYAN)
LOGGER_COMPONENT_MAPPING_INFO(_DEBUG, UDP, ::util::format::Color::LIGHT_GRAY)
LOGGER_COMPONENT_MAPPING_INFO(_DEBUG, LWIP, ::util::format::Color::LIGHT_YELLOW)
#endif // PLATFORM_SUPPORT_ETHERNET
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

using ::util::logger::Logger;
using ::util::logger::LWIP;

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

    Logger::info(LWIP, buffer);
}

extern "C" void log_lwipError(char const* message) { Logger::error(LWIP, message); }

} // namespace logger
