#pragma once

#include <vector>

#include <gmock/gmock.h>

#include "middleware/logger/Logger.h"

namespace middleware
{
namespace logger
{
namespace test
{
namespace mock
{

class LoggerMock
{
public:
    LoggerMock();
    ~LoggerMock();

    MOCK_METHOD(void, log, (LogLevel const, char const* const, std::vector<uint32_t> const&));
    MOCK_METHOD(void, log_binary, (LogLevel const, etl::span<uint8_t const> const));
    MOCK_METHOD(uint32_t, getMessageId, (Error const));
};

} // namespace mock
} // namespace test
} // namespace logger
} // namespace middleware
