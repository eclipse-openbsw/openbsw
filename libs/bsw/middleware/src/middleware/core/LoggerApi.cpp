// Copyright 2025 BMW AG

#include "middleware/core/LoggerApi.h"

#include <cstddef>
#include <cstdint>

#include <etl/array.h>
#include <etl/byte_stream.h>

#include "middleware/core/Message.h"
#include "middleware/core/types.h"
#include "middleware/logger/Logger.h"

namespace middleware
{
namespace logger
{
namespace
{

void serialize(etl::byte_stream_writer& writer, uint8_t const value)
{
    writer.write_unchecked(value);
}

void serialize(etl::byte_stream_writer& writer, uint16_t const value)
{
    writer.write_unchecked(value);
}

void serialize(etl::byte_stream_writer& writer, uint32_t const value)
{
    writer.write_unchecked(value);
}

void serialize(etl::byte_stream_writer& writer, core::Message const& value)
{
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    static_assert(
        count_bytes<core::Message>::VALUE == 10U, "Message log size in bytes exceeds the payload");

    writer.write_unchecked(value.getHeader().srcClusterId);
    writer.write_unchecked(value.getHeader().tgtClusterId);
    writer.write_unchecked(value.getHeader().serviceId);
    writer.write_unchecked(value.getHeader().serviceInstanceId);
    writer.write_unchecked(value.getHeader().memberId);
    writer.write_unchecked(value.getHeader().requestId);
}

template<typename Value, typename... Values>
void serialize(etl::byte_stream_writer& writer, Value value, Values... values)
{
    serialize(writer, value);
    serialize(writer, values...);
}

template<uint32_t MAX_SIZE, typename... Values>
void serialize(etl::byte_stream_writer& writer, Values... values)
{
    static_assert(
        count_bytes<Values...>::VALUE == MAX_SIZE, "Total size in bytes exceeds the payload");

    serialize(writer, values...);
}
} // namespace

void logAllocationFailure(
    LogLevel const level,
    Error const error,
    core::HRESULT const res,
    core::Message const& msg,
    uint32_t const size)
{
    static char const* const kformat = "e:%d r:%d SC:%d TC:%d S:%d I:%d M:%d R:%d s:%d";

    etl::array<uint8_t, ALLOCATION_FAILURE_LOG_SIZE> temp{};
    etl::byte_stream_writer writer{temp, etl::endian::native};
    serialize<ALLOCATION_FAILURE_LOG_SIZE>(
        writer,
        getMessageId(error),
        static_cast<uint8_t>(error),
        static_cast<uint8_t>(res),
        msg,
        size);

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    middleware::logger::log(
        level,
        kformat,
        error,
        res,
        msg.getHeader().srcClusterId,
        msg.getHeader().tgtClusterId,
        msg.getHeader().serviceId,
        msg.getHeader().serviceInstanceId,
        msg.getHeader().memberId,
        msg.getHeader().requestId,
        size);
    middleware::logger::log_binary(level, temp);
}

void logInitFailure(
    LogLevel const level,
    Error const error,
    core::HRESULT const res,
    uint16_t const serviceId,
    uint16_t const serviceInstanceId,
    uint8_t const sourceCluster)
{
    static char const* const kformat = "e:%d r:%d SC:%d S:%d I:%d";

    etl::array<uint8_t, INIT_FAILURE_LOG_SIZE>
        temp{}; // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    etl::byte_stream_writer writer{temp, etl::endian::native};
    serialize<INIT_FAILURE_LOG_SIZE>(
        writer,
        getMessageId(error),
        static_cast<uint8_t>(error),
        static_cast<uint8_t>(res),
        static_cast<uint8_t>(sourceCluster),
        static_cast<uint16_t>(serviceId),
        static_cast<uint16_t>(serviceInstanceId));

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    middleware::logger::log(
        level, kformat, error, res, sourceCluster, serviceId, serviceInstanceId);
    middleware::logger::log_binary(level, temp);
}

void logMessageSendingFailure(
    LogLevel const level, Error const error, core::HRESULT const res, core::Message const& msg)
{
    static char const* const kformat = "e:%d r:%d SC:%d TC:%d S:%d I:%d M:%d R:%d";

    etl::array<uint8_t, MSG_SEND_FAILURE_LOG_SIZE>
        temp{}; // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    etl::byte_stream_writer writer{temp, etl::endian::native};
    serialize<MSG_SEND_FAILURE_LOG_SIZE>(
        writer, getMessageId(error), static_cast<uint8_t>(error), static_cast<uint8_t>(res), msg);

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    middleware::logger::log(
        level,
        kformat,
        error,
        res,
        msg.getHeader().srcClusterId,
        msg.getHeader().tgtClusterId,
        msg.getHeader().serviceId,
        msg.getHeader().serviceInstanceId,
        msg.getHeader().memberId,
        msg.getHeader().requestId);
    middleware::logger::log_binary(level, temp);
}

void logCrossThreadViolation(
    LogLevel const level,
    Error const error,
    uint8_t const sourceCluster,
    uint16_t const serviceId,
    uint16_t const serviceInstanceId,
    uint32_t const initId,
    uint32_t const currentTaskId)
{
    static char const* const kformat = "e:%d SC:%d S:%d I:%d T0:%d T1:%d";

    etl::array<uint8_t, CROSS_THREAD_VIOLATION_LOG_SIZE>
        temp{}; // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    etl::byte_stream_writer writer{temp, etl::endian::native};
    serialize<CROSS_THREAD_VIOLATION_LOG_SIZE>(
        writer,
        getMessageId(error),
        static_cast<uint8_t>(error),
        static_cast<uint8_t>(sourceCluster),
        static_cast<uint16_t>(serviceId),
        static_cast<uint16_t>(serviceInstanceId),
        static_cast<uint32_t>(initId),
        static_cast<uint32_t>(currentTaskId));

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    middleware::logger::log(
        level, kformat, error, sourceCluster, serviceId, serviceInstanceId, initId, currentTaskId);
    middleware::logger::log_binary(level, temp);
}

} // namespace logger
} // namespace middleware
