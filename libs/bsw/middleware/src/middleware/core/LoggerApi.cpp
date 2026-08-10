/********************************************************************************
 * Copyright (c) 2025 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "middleware/core/LoggerApi.h"

#include "middleware/core/Message.h"
#include "middleware/core/types.h"
#include "middleware/logger/Logger.h"

#include <etl/array.h>
#include <etl/byte_stream.h>

#include <cstddef>
#include <cstdint>

namespace middleware::logger
{
namespace
{

void serialize(::etl::byte_stream_writer& writer, uint8_t const value)
{
    writer.write_unchecked(value);
}

void serialize(::etl::byte_stream_writer& writer, uint16_t const value)
{
    writer.write_unchecked(value);
}

void serialize(::etl::byte_stream_writer& writer, uint32_t const value)
{
    writer.write_unchecked(value);
}

void serialize(::etl::byte_stream_writer& writer, core::Message const& value)
{
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    static_assert(
        CountBytes<core::Message>::VALUE == 10U, "Message log size in bytes exceeds the payload");

    writer.write_unchecked(value.getHeader().srcClusterId);
    writer.write_unchecked(value.getHeader().tgtClusterId);
    writer.write_unchecked(value.getHeader().serviceId);
    writer.write_unchecked(value.getHeader().serviceInstanceId);
    writer.write_unchecked(value.getHeader().memberId);
    writer.write_unchecked(value.getHeader().requestId);
}

template<typename Value, typename... Values>
void serialize(::etl::byte_stream_writer& writer, Value value, Values... values)
{
    serialize(writer, value);
    serialize(writer, values...);
}

template<uint32_t MAX_SIZE, typename... Values>
void serialize(::etl::byte_stream_writer& writer, Values... values)
{
    static_assert(
        CountBytes<Values...>::VALUE == MAX_SIZE, "Total size in bytes exceeds the payload");

    serialize(writer, values...);
}

template<uint32_t Size, typename... Args>
void logBinaryRecord(LogLevel const level, Args const&... args)
{
    ::etl::array<uint8_t, Size> temp{};
    ::etl::byte_stream_writer writer{temp, ::etl::endian::native};
    serialize<Size>(writer, args...);
    middleware::logger::logBinary(level, temp);
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

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    middleware::logger::log(
        level,
        kformat,
        static_cast<unsigned int>(error),
        static_cast<unsigned int>(res),
        msg.getHeader().srcClusterId,
        msg.getHeader().tgtClusterId,
        msg.getHeader().serviceId,
        msg.getHeader().serviceInstanceId,
        msg.getHeader().memberId,
        msg.getHeader().requestId,
        size);

    logBinaryRecord<ALLOCATION_FAILURE_LOG_SIZE>(
        level,
        getMessageId(error),
        static_cast<uint8_t>(error),
        static_cast<uint8_t>(res),
        msg,
        size);
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

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    middleware::logger::log(
        level,
        kformat,
        static_cast<unsigned int>(error),
        static_cast<unsigned int>(res),
        sourceCluster,
        serviceId,
        serviceInstanceId);

    logBinaryRecord<INIT_FAILURE_LOG_SIZE>(
        level,
        getMessageId(error),
        static_cast<uint8_t>(error),
        static_cast<uint8_t>(res),
        static_cast<uint8_t>(sourceCluster),
        static_cast<uint16_t>(serviceId),
        static_cast<uint16_t>(serviceInstanceId));
}

void logMessageSendingFailure(
    LogLevel const level, Error const error, core::HRESULT const res, core::Message const& msg)
{
    static char const* const kformat = "e:%d r:%d SC:%d TC:%d S:%d I:%d M:%d R:%d";

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    middleware::logger::log(
        level,
        kformat,
        static_cast<unsigned int>(error),
        static_cast<unsigned int>(res),
        msg.getHeader().srcClusterId,
        msg.getHeader().tgtClusterId,
        msg.getHeader().serviceId,
        msg.getHeader().serviceInstanceId,
        msg.getHeader().memberId,
        msg.getHeader().requestId);

    logBinaryRecord<MSG_SEND_FAILURE_LOG_SIZE>(
        level, getMessageId(error), static_cast<uint8_t>(error), static_cast<uint8_t>(res), msg);
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

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    middleware::logger::log(
        level,
        kformat,
        static_cast<unsigned int>(error),
        sourceCluster,
        serviceId,
        serviceInstanceId,
        initId,
        currentTaskId);

    logBinaryRecord<CROSS_THREAD_VIOLATION_LOG_SIZE>(
        level,
        getMessageId(error),
        static_cast<uint8_t>(error),
        static_cast<uint8_t>(sourceCluster),
        static_cast<uint16_t>(serviceId),
        static_cast<uint16_t>(serviceInstanceId),
        static_cast<uint32_t>(initId),
        static_cast<uint32_t>(currentTaskId));
}

void logFrameFailure(LogLevel const level, Error const error, uint32_t const frameId)
{
    static char const* const kformat = "e:%d ID:0x%x";

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    middleware::logger::log(level, kformat, static_cast<unsigned int>(error), frameId);
    logBinaryRecord<FRAME_FAILURE_LOG_SIZE>(
        level, getMessageId(error), static_cast<uint8_t>(error), static_cast<uint32_t>(frameId));
}

void logPduFailure(LogLevel const level, Error const error, uint32_t const pduId)
{
    static constexpr char kdecimalFormat[] = "e:%d ID:%u";
    static constexpr char khexFormat[]     = "e:%d ID:0x%x";
    char const* const kformat
        = (error == Error::PduRouteUnknown || error == Error::PduPayloadAllocation) ? kdecimalFormat
                                                                                    : khexFormat;

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    middleware::logger::log(level, kformat, static_cast<unsigned int>(error), pduId);
    logBinaryRecord<PDU_FAILURE_LOG_SIZE>(
        level, getMessageId(error), static_cast<uint8_t>(error), static_cast<uint32_t>(pduId));
}

void logPduBroadcastFailure(
    LogLevel const level, Error const error, uint32_t const pduId, uint8_t const clusterId)
{
    static char const* const kformat = "e:%d ID:%u C:%u";

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    middleware::logger::log(level, kformat, static_cast<unsigned int>(error), pduId, clusterId);
    logBinaryRecord<PDU_BROADCAST_FAILURE_LOG_SIZE>(
        level,
        getMessageId(error),
        static_cast<uint8_t>(error),
        static_cast<uint32_t>(pduId),
        static_cast<uint8_t>(clusterId));
}

void logServiceMemberMappingNotFound(
    LogLevel const level, Error const error, uint16_t const serviceId, uint16_t const memberId)
{
    static char const* const kformat = "e:%d S:%u M:%u";

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    middleware::logger::log(level, kformat, static_cast<unsigned int>(error), serviceId, memberId);
    logBinaryRecord<SERVICE_MEMBER_MAPPING_NOT_FOUND_LOG_SIZE>(
        level,
        getMessageId(error),
        static_cast<uint8_t>(error),
        static_cast<uint16_t>(serviceId),
        static_cast<uint16_t>(memberId));
}

void logPduPayloadOutOfBounds(
    LogLevel const level, Error const error, uint32_t const pduId, uint32_t const maxBytes)
{
    static char const* const kformat = "e:%d ID:%u MAX:%u";

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    middleware::logger::log(level, kformat, static_cast<unsigned int>(error), pduId, maxBytes);
    logBinaryRecord<PDU_PAYLOAD_OUT_OF_BOUNDS_LOG_SIZE>(
        level,
        getMessageId(error),
        static_cast<uint8_t>(error),
        static_cast<uint32_t>(pduId),
        static_cast<uint32_t>(maxBytes));
}

} // namespace middleware::logger
