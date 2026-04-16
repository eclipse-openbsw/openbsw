#include "LoggerMock.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <vector>

#include "middleware/logger/Logger.h"

namespace middleware::logger::test::mock
{
namespace
{
middleware::logger::test::mock::LoggerMock* _loggerMockPtr{nullptr};
} // namespace

LoggerMock::LoggerMock() { _loggerMockPtr = this; }

LoggerMock::~LoggerMock() { _loggerMockPtr = nullptr; }

} // namespace middleware::logger::test::mock

namespace middleware::logger
{

void log(LogLevel const level, char const* const format, ...) // NOLINT(cert-dcl50-cpp)
{
    va_list ap, ap_copy;
    va_start(ap, format);
    va_copy(ap_copy, ap);
    vprintf(format, ap_copy);
    va_end(ap_copy);
    printf("\n");

    std::vector<uint32_t> args;
    for (char const* p = format; *p != '\0'; ++p)
    {
        switch (*p)
        {
            case '%':
                switch (*++p)
                {
                    case 'd': args.push_back(static_cast<uint32_t>(va_arg(ap, int))); continue;
                    case 'u':
                        args.push_back(static_cast<uint32_t>(va_arg(ap, unsigned int)));
                        continue;
                    default: break;
                }
                break;
            default: break;
        }
    }

    if (test::mock::_loggerMockPtr != nullptr)
    {
        test::mock::_loggerMockPtr->log(level, format, args);
    }

    va_end(ap);
}

void logBinary(LogLevel const level, etl::span<uint8_t const> const data)
{
    for (size_t i = 0; i < data.size(); ++i)
    {
        printf("%d ", data[i]);
    }
    printf("\n");

    if (test::mock::_loggerMockPtr != nullptr)
    {
        test::mock::_loggerMockPtr->logBinary(level, data);
    }
}

uint32_t getMessageId(Error const id)
{
    if (test::mock::_loggerMockPtr != nullptr)
    {
        return test::mock::_loggerMockPtr->getMessageId(id);
    }

    return etl::numeric_limits<uint32_t>::max();
}

} // namespace middleware::logger
