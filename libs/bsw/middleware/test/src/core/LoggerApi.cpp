/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include <etl/limits.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "logger/DslLogger.h"
#include "middleware/core/LoggerApi.h"

namespace middleware::core::test
{

class LoggerApiTest : public ::testing::Test
{
public:
    void SetUp() override { _loggerMock.setup(); }

    void TearDown() override { _loggerMock.teardown(); }

protected:
    middleware::logger::test::DslLogger _loggerMock{};
};

TEST_F(LoggerApiTest, TestLogAllocationFailure)
{
    // ARRANGE
    logger::LogLevel const level = logger::LogLevel::Error;
    logger::Error const error    = logger::Error::Allocation;
    HRESULT const res            = HRESULT::CannotAllocatePayload;
    core::Message const msg
        = core::Message::createRequest(0x1000U, 0x2000U, 0x3000U, 0x4000U, 1U, 2U, 3U);

    // ACT && ASSERT
    _loggerMock.EXPECT_LOG(
        level,
        "e:%d r:%d SC:%d TC:%d S:%d I:%d M:%d R:%d s:%d",
        error,
        res,
        msg.getHeader().srcClusterId,
        msg.getHeader().tgtClusterId,
        msg.getHeader().serviceId,
        msg.getHeader().serviceInstanceId,
        msg.getHeader().memberId,
        msg.getHeader().requestId,
        static_cast<uint32_t>(sizeof(msg)));
    _loggerMock.EXPECT_EVENT_LOG(
        logger::LogLevel::Error,
        error,
        res,
        msg.getHeader().srcClusterId,
        msg.getHeader().tgtClusterId,
        msg.getHeader().serviceId,
        msg.getHeader().serviceInstanceId,
        msg.getHeader().memberId,
        msg.getHeader().requestId,
        static_cast<uint32_t>(sizeof(msg)));
    middleware::logger::logAllocationFailure(level, error, res, msg, sizeof(msg));
}

TEST_F(LoggerApiTest, TestLogInitFailure)
{
    // ARRANGE
    logger::LogLevel const level     = logger::LogLevel::Critical;
    logger::Error const error        = logger::Error::ProxyInitialization;
    const HRESULT res                = HRESULT::TransceiverInitializationFailed;
    uint16_t const serviceId         = ::etl::numeric_limits<uint16_t>::max();
    uint16_t const serviceInstanceId = ::etl::numeric_limits<uint16_t>::max();
    uint8_t const sourceCluster      = ::etl::numeric_limits<uint8_t>::max();

    // ACT && ASSERT
    _loggerMock.EXPECT_LOG(
        level,
        "e:%d r:%d SC:%d S:%d I:%d",
        error,
        res,
        sourceCluster,
        serviceId,
        serviceInstanceId);
    _loggerMock.EXPECT_EVENT_LOG(level, error, res, sourceCluster, serviceId, serviceInstanceId);
    middleware::logger::logInitFailure(
        level, error, res, serviceId, serviceInstanceId, sourceCluster);
}

TEST_F(LoggerApiTest, TestLogMessageSendingFailure)
{
    // ARRANGE
    logger::LogLevel const level = logger::LogLevel::Error;
    logger::Error const error    = logger::Error::DispatchMessage;
    HRESULT const res            = HRESULT::ServiceNotFound;
    core::Message const msg
        = core::Message::createRequest(0x1000U, 0x2000U, 0x3000U, 0x4000U, 1U, 2U, 3U);

    // ACT && ASSERT
    _loggerMock.EXPECT_LOG(
        level,
        "e:%d r:%d SC:%d TC:%d S:%d I:%d M:%d R:%d",
        error,
        res,
        msg.getHeader().srcClusterId,
        msg.getHeader().tgtClusterId,
        msg.getHeader().serviceId,
        msg.getHeader().serviceInstanceId,
        msg.getHeader().memberId,
        msg.getHeader().requestId);
    _loggerMock.EXPECT_EVENT_LOG(
        logger::LogLevel::Error,
        error,
        res,
        msg.getHeader().srcClusterId,
        msg.getHeader().tgtClusterId,
        msg.getHeader().serviceId,
        msg.getHeader().serviceInstanceId,
        msg.getHeader().memberId,
        msg.getHeader().requestId);
    middleware::logger::logMessageSendingFailure(level, error, res, msg);
}

TEST_F(LoggerApiTest, TestLogCrossThreadViolation)
{
    // ARRANGE
    logger::LogLevel const level     = logger::LogLevel::Critical;
    logger::Error const error        = logger::Error::ProxyCrossThreadViolation;
    uint16_t const serviceId         = ::etl::numeric_limits<uint16_t>::max();
    uint16_t const serviceInstanceId = ::etl::numeric_limits<uint16_t>::max();
    uint8_t const sourceCluster      = ::etl::numeric_limits<uint8_t>::max();
    uint32_t const initId            = ::etl::numeric_limits<uint32_t>::max();
    uint32_t const currentTaskId     = ::etl::numeric_limits<uint32_t>::max();

    // ACT && ASSERT
    _loggerMock.EXPECT_LOG(
        level,
        "e:%d SC:%d S:%d I:%d T0:%d T1:%d",
        error,
        sourceCluster,
        serviceId,
        serviceInstanceId,
        initId,
        currentTaskId);
    _loggerMock.EXPECT_EVENT_LOG(
        level, error, sourceCluster, serviceId, serviceInstanceId, initId, currentTaskId);
    middleware::logger::logCrossThreadViolation(
        level, error, sourceCluster, serviceId, serviceInstanceId, initId, currentTaskId);
}

TEST_F(LoggerApiTest, TestLogFrameIdUnknown)
{
    // ARRANGE
    logger::LogLevel const level = logger::LogLevel::Error;
    logger::Error const error    = logger::Error::FrameIdUnknown;
    uint32_t const frameId       = ::etl::numeric_limits<uint32_t>::max();

    // ACT && ASSERT
    _loggerMock.EXPECT_LOG(level, "e:%d ID:0x%x", error, frameId);
    _loggerMock.EXPECT_EVENT_LOG(level, error, frameId);
    middleware::logger::logFrameFailure(level, error, frameId);
}

TEST_F(LoggerApiTest, TestLogCanConversionFailureForFrameToPdu)
{
    // ARRANGE
    logger::LogLevel const level = logger::LogLevel::Error;
    logger::Error const error    = logger::Error::FrameToPduConversion;
    uint32_t const frameId       = ::etl::numeric_limits<uint32_t>::max();

    // ACT && ASSERT
    _loggerMock.EXPECT_LOG(level, "e:%d ID:0x%x", error, frameId);
    _loggerMock.EXPECT_EVENT_LOG(level, error, frameId);
    middleware::logger::logFrameFailure(level, error, frameId);
}

TEST_F(LoggerApiTest, TestLogPduFailureForFrameSlotOutOfBounds)
{
    // ARRANGE
    logger::LogLevel const level = logger::LogLevel::Error;
    logger::Error const error    = logger::Error::FrameSlotOutOfBounds;
    uint32_t const pduId         = ::etl::numeric_limits<uint32_t>::max();

    // ACT && ASSERT
    _loggerMock.EXPECT_LOG(level, "e:%d ID:0x%x", error, pduId);
    _loggerMock.EXPECT_EVENT_LOG(level, error, pduId);
    middleware::logger::logPduFailure(level, error, pduId);
}

TEST_F(LoggerApiTest, TestLogCanConversionFailureForPduToFrame)
{
    // ARRANGE
    logger::LogLevel const level = logger::LogLevel::Error;
    logger::Error const error    = logger::Error::PduToFrameConversion;
    uint32_t const pduId         = ::etl::numeric_limits<uint32_t>::max();

    // ACT && ASSERT
    _loggerMock.EXPECT_LOG(level, "e:%d ID:0x%x", error, pduId);
    _loggerMock.EXPECT_EVENT_LOG(level, error, pduId);
    middleware::logger::logPduFailure(level, error, pduId);
}

TEST_F(LoggerApiTest, TestLogPduFailureForRouteUnknown)
{
    // ARRANGE
    logger::LogLevel const level = logger::LogLevel::Error;
    logger::Error const error    = logger::Error::PduRouteUnknown;
    uint32_t const pduId         = ::etl::numeric_limits<uint32_t>::max();

    // ACT && ASSERT
    _loggerMock.EXPECT_LOG(level, "e:%d ID:%u", error, pduId);
    _loggerMock.EXPECT_EVENT_LOG(level, error, pduId);
    middleware::logger::logPduFailure(level, error, pduId);
}

TEST_F(LoggerApiTest, TestLogPduFailureForPayloadAllocation)
{
    // ARRANGE
    logger::LogLevel const level = logger::LogLevel::Error;
    logger::Error const error    = logger::Error::PduPayloadAllocation;
    uint32_t const pduId         = ::etl::numeric_limits<uint32_t>::max();

    // ACT && ASSERT
    _loggerMock.EXPECT_LOG(level, "e:%d ID:%u", error, pduId);
    _loggerMock.EXPECT_EVENT_LOG(level, error, pduId);
    middleware::logger::logPduFailure(level, error, pduId);
}

TEST_F(LoggerApiTest, TestLogPduBroadcastFailure)
{
    // ARRANGE
    logger::LogLevel const level = logger::LogLevel::Info;
    logger::Error const error    = logger::Error::PduBroadcast;
    uint32_t const pduId         = ::etl::numeric_limits<uint32_t>::max();
    uint8_t const clusterId      = ::etl::numeric_limits<uint8_t>::max();

    // ACT && ASSERT
    _loggerMock.EXPECT_LOG(level, "e:%d ID:%u C:%u", error, pduId, clusterId);
    _loggerMock.EXPECT_EVENT_LOG(level, error, pduId, clusterId);
    middleware::logger::logPduBroadcastFailure(level, error, pduId, clusterId);
}

TEST_F(LoggerApiTest, TestLogServiceMemberMappingNotFound)
{
    // ARRANGE
    logger::LogLevel const level = logger::LogLevel::Warning;
    logger::Error const error    = logger::Error::ServiceMemberMappingNotFound;
    uint16_t const serviceId     = ::etl::numeric_limits<uint16_t>::max();
    uint16_t const memberId      = ::etl::numeric_limits<uint16_t>::max();

    // ACT && ASSERT
    _loggerMock.EXPECT_LOG(level, "e:%d S:%u M:%u", error, serviceId, memberId);
    _loggerMock.EXPECT_EVENT_LOG(level, error, serviceId, memberId);
    middleware::logger::logServiceMemberMappingNotFound(level, error, serviceId, memberId);
}

TEST_F(LoggerApiTest, TestLogPduFailureForOffsetOutOfBounds)
{
    // ARRANGE
    logger::LogLevel const level = logger::LogLevel::Error;
    logger::Error const error    = logger::Error::PduOffsetOutOfBounds;
    uint32_t const pduId         = ::etl::numeric_limits<uint32_t>::max();

    // ACT && ASSERT
    _loggerMock.EXPECT_LOG(level, "e:%d ID:0x%x", error, pduId);
    _loggerMock.EXPECT_EVENT_LOG(level, error, pduId);
    middleware::logger::logPduFailure(level, error, pduId);
}

TEST_F(LoggerApiTest, TestLogPduPayloadOutOfBounds)
{
    // ARRANGE
    logger::LogLevel const level = logger::LogLevel::Error;
    logger::Error const error    = logger::Error::PduPayloadOutOfBounds;
    uint32_t const pduId         = ::etl::numeric_limits<uint32_t>::max();
    uint32_t const maxBytes      = ::etl::numeric_limits<uint32_t>::max();

    // ACT && ASSERT
    _loggerMock.EXPECT_LOG(level, "e:%d ID:%u MAX:%u", error, pduId, maxBytes);
    _loggerMock.EXPECT_EVENT_LOG(level, error, pduId, maxBytes);
    middleware::logger::logPduPayloadOutOfBounds(level, error, pduId, maxBytes);
}

} // namespace middleware::core::test
