// Copyright 2024 Accenture.

#include "uds/UdsLogger.h"
#include "util/logger/TestConsoleLogger.h"

DEFINE_LOGGER_COMPONENT(GLOBAL)

using namespace ::util::logger;

LoggerComponentInfo components[]
    = {LoggerComponentInfo(GLOBAL, "GLOBAL", LEVEL_DEBUG),
       LoggerComponentInfo(UDS, "UDS", LEVEL_DEBUG)};

TestConsoleLogger logger(components);
