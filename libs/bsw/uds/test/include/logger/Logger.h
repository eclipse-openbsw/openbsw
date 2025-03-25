// Copyright 2024 Accenture.

#pragma once

#include "platform/estdint.h"

namespace logger
{
class ILoggerOutput;

enum Component
{
    FIRST_LOGGER_COMPONENT,
    _BLOCKTRANSFERSERVER = FIRST_LOGGER_COMPONENT,
    _BSP,
    _CAN,
    _COMMON,
    _DEM,
    _DIAGMASTER,
    _DIAGNOSIS,
    _DMCLIENT,
    _EEPROM,
    _ETHERNET,
    _FLEXRAY,
    _FRTRANSCEIVER,
    _FUSI,
    _FZM,
    _HSFZ,
    _HTTP,
    _LIFECYCLE,
    _LIN,
    _MCAL,
    _MSM,
    _RPC,
    _SCCLIENT,
    _SYSTIMECLIENT,
    _TAS,
    _TCP,
    _TEST,
    _TPROUTER,
    _TRANSPORT,
    _UDP,
    _UGW,
    _VCM,
    _GLOBAL,
    NUMBER_OF_LOGGER_COMPONENTS
};

enum Level
{
    _NOLOGGING,
    _DEBUG,
    _INFO,
    _WARN,
    _ERROR,
    _CRITICAL
};

class Logger
{
public:
    static void log(Component component, Level level, char const* str, ...);

    static void init(ILoggerOutput& loggerOutput) {}

    static void setLevelForAll(Level level) {}
};

} // namespace logger

