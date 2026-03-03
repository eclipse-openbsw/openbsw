#pragma once

#include <cstdarg>
#include <cstdint>
#include <cstring>
#include <string>

#include <etl/array.h>
#include <etl/span.h>
#include <etl/vector.h>
#include <gmock/gmock.h>

#include "middleware/core/LoggerApi.h"
#include "middleware/logger/Logger.h"
#include "mock/LoggerMock.h"

namespace middleware
{
namespace logger
{
namespace test
{

using ::testing::_;
using ::testing::Cardinality;
using ::testing::ElementsAreArray;
using ::testing::Exactly;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::StrEq;

class DslLogger
{
public:
    NiceMock<mock::LoggerMock> _mock;

    void setup() {}

    void teardown() {}

    template<typename... Args>
    void EXPECT_LOG(LogLevel const level, std::string const& format, Args... args)
    {
        etl::vector<uint32_t, sizeof...(Args)> vec;
        push_all(vec, args...);

        EXPECT_CALL(_mock, log(level, StrEq(format.c_str()), ElementsAreArray(vec)))
            .Times(Exactly(1U))
            .WillRepeatedly(Return());
    }

    template<typename... Args>
    void EXPECT_EVENT_LOG(LogLevel const level, Error const error, Args... args)
    {
        static etl::array<uint8_t, sizeof(uint32_t) + sizeof(Error) + count_bytes<Args...>::VALUE>
            buffer{};

        uint32_t const messageId = logger::getMessageId(error);

        uint32_t index = 0U;
        copy_to_buffer(buffer.data(), index, messageId);
        copy_to_buffer(buffer.data(), index, error);
        copy_to_buffer(buffer.data(), index, args...);

        EXPECT_CALL(_mock, log_binary(level, ElementsAreArray(buffer)))
            .Times(Exactly(1U))
            .WillRepeatedly(Return());
    }

    void EXPECT_NO_LOG() { EXPECT_CALL(_mock, log(_, _, _)).Times(0); }

    void EXPECT_NO_BINARY_LOG() { EXPECT_CALL(_mock, log_binary(_, _)).Times(0); }

    void EXPECT_NO_LOGGING()
    {
        EXPECT_NO_LOG();
        EXPECT_NO_BINARY_LOG();
    }

private:
    template<typename T>
    void push_all(etl::ivector<uint32_t>& vec, T arg)
    {
        vec.push_back(static_cast<uint32_t>(arg));
    }

    template<typename T, typename... Args>
    void push_all(etl::ivector<uint32_t>& vec, T arg, Args... args)
    {
        vec.push_back(static_cast<uint32_t>(arg));
        push_all(vec, args...);
    }

    template<typename T>
    void copy_to_buffer(uint8_t* const buffer, uint32_t& index, T arg)
    {
        memcpy(&buffer[index], &arg, sizeof(arg));
        index += sizeof(arg);
    }

    template<typename T, typename... Args>
    void copy_to_buffer(uint8_t* const buffer, uint32_t& index, T arg, Args... args)
    {
        memcpy(&buffer[index], &arg, sizeof(arg));
        index += sizeof(arg);
        copy_to_buffer(buffer, index, args...);
    }
};

} // namespace test
} // namespace logger
} // namespace middleware
