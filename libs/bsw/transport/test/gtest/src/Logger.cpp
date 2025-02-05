// Copyright 2024 Accenture.

#include "transport/TransportLogger.h"

#include <util/logger/TestConsoleLogger.h>

using namespace ::util::logger;

LoggerComponentInfo components[] = {LoggerComponentInfo(TRANSPORT, "TRANSPORT", LEVEL_DEBUG)};

TestConsoleLogger logger(components);
