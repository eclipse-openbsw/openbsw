/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/
#pragma once

#include <cstdint>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>

namespace simulation
{

/**
 * Centralized thread-safe simulation logger class
 */
class Logger
{
public:
    /**
     * Log function with cluster ID
     * Accepts multiple arguments and builds the message internally
     * @param clusterId The cluster ID
     * @param args Arguments to be concatenated into the log message
     */
    template<typename... Args>
    static void log(uint8_t clusterId, Args&&... args)
    {
        std::ostringstream oss;
        oss << getTimestamp() << getClusterId(clusterId);
        ((oss << args), ...); // C++17 fold expression
        writeLog(oss.str());
    }

    /**
     * Statistics logging function with cluster ID
     * Accepts multiple arguments and builds the message internally
     * @param clusterId The cluster ID
     * @param args Arguments to be concatenated into the log message
     */
    template<typename... Args>
    static void logStats(uint8_t clusterId, Args&&... args)
    {
        std::ostringstream oss;
        oss << getTimestamp() << getClusterId(clusterId);
        ((oss << args), ...); // C++17 fold expression
        writeLog(oss.str());
    }

    /**
     * General message logging
     * Accepts multiple arguments and builds the message internally
     * @param args Arguments to be concatenated into the log message
     */
    template<typename... Args>
    static void log(Args&&... args)
    {
        std::ostringstream oss;
        ((oss << args), ...); // C++17 fold expression
        writeLog(oss.str());
    }

private:
    static std::mutex logMutex_;
    static void writeLog(std::string const& logLine);
    static std::string getTimestamp();
    static std::string getClusterId(uint8_t const id);
};

} // namespace simulation
