/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/
#include "Logger.h"

#include <chrono>
#include <iomanip>

namespace simulation
{

std::mutex Logger::logMutex_;

void Logger::writeLog(std::string const& logLine)
{
    std::lock_guard<std::mutex> lock(logMutex_);
    std::cout << logLine << std::endl;
}

std::string Logger::getTimestamp()
{
    std::time_t now = std::time(nullptr);
    std::tm tm{};
    localtime_r(&now, &tm);
    std::ostringstream oss;
    oss << "[" << std::put_time(&tm, "%T") << "] ";
    return oss.str();
}

std::string Logger::getClusterId(uint8_t const id)
{
    std::ostringstream oss;
    oss << "[Cluster" << static_cast<int>(id) << "] ";
    return oss.str();
}

} // namespace simulation
