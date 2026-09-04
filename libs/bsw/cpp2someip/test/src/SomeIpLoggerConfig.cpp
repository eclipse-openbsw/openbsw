/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/logger.h"

#include <util/logger/TestConsoleLogger.h>

using namespace ::util::logger;

static LoggerComponentInfo _someipLoggerComponents[] = {
    LoggerComponentInfo(SOMEIP, "SOMEIP", LEVEL_INFO),
};

static TestConsoleLogger _someipLogger(_someipLoggerComponents);
