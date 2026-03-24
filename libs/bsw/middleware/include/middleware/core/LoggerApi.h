// Copyright 2025 BMW AG

#pragma once

#include "middleware/core/Message.h"
#include "middleware/core/types.h"
#include "middleware/logger/Logger.h"

#include <etl/platform.h>

#include <cstdint>

namespace middleware
{
namespace logger
{

/** Log buffer size for allocation failures. */
ETL_INLINE_VAR constexpr uint8_t ALLOCATION_FAILURE_LOG_SIZE     = 20U;
/** Log buffer size for initialization failures. */
ETL_INLINE_VAR constexpr uint8_t INIT_FAILURE_LOG_SIZE           = 11U;
/** Log buffer size for message sending failures. */
ETL_INLINE_VAR constexpr uint8_t MSG_SEND_FAILURE_LOG_SIZE       = 16U;
/** Log buffer size for cross-thread violations. */
ETL_INLINE_VAR constexpr uint8_t CROSS_THREAD_VIOLATION_LOG_SIZE = 18U;

/**
 * Helper template to count the total size of types in bytes.
 * This template struct recursively calculates the total byte size of multiple types,
 * used for determining log buffer sizes. Specialized for core::Message to only count relevant
 * header information.
 *
 * \tparam Args variadic template parameter pack of types to count
 */
template<typename... Args>
struct CountBytes;

/**
 * Specialization for a single type.
 * \tparam T the type to count bytes for
 */
template<typename T>
struct CountBytes<T>
{
    /** The size of type T in bytes. */
    static constexpr size_t VALUE = sizeof(T);
};

/**
 * Specialization for core::Message type.
 * Counts only the bytes of the header fields actually serialized:
 * srcClusterId, tgtClusterId, serviceId, serviceInstanceId, memberId, requestId.
 */
template<>
struct CountBytes<core::Message>
{
    using Header = core::Message::Header;
    static constexpr size_t VALUE
        = sizeof(decltype(Header::srcClusterId)) + sizeof(decltype(Header::tgtClusterId))
          + sizeof(decltype(Header::serviceId)) + sizeof(decltype(Header::serviceInstanceId))
          + sizeof(decltype(Header::memberId)) + sizeof(decltype(Header::requestId));
};

/**
 * Recursive template specialization for multiple types.
 * \tparam T the first type
 * \tparam Args the remaining types
 */
template<typename T, typename... Args>
struct CountBytes<T, Args...>
{
    /** Sum of all type sizes. */
    static constexpr size_t VALUE = CountBytes<T>::VALUE + CountBytes<Args...>::VALUE;
};

/**
 * Logs an allocation failure event.
 * Records an allocation failure with the specified severity level, error code, result,
 * message details, and the size of the failed allocation. This is typically called when memory
 * allocation for message handling fails.
 *
 * \param level the severity level of the log message (e.g., INFO, WARN, ERROR)
 * \param error the specific error that occurred during allocation
 * \param res the HRESULT indicating the result of the allocation attempt
 * \param msg constant reference to the message object containing middleware communication details
 * \param size the size of the allocation that failed
 */
void logAllocationFailure(
    LogLevel level, Error error, core::HRESULT res, core::Message const& msg, uint32_t size);

/**
 * Logs an initialization failure event for a service.
 * Records a service initialization failure with the specified severity level, error code,
 * result, and service identification information. This is called when a service or service instance
 * fails to initialize properly.
 *
 * \param level the severity level of the log message
 * \param error the specific error encountered during the initialization process
 * \param res the HRESULT indicating the result of the initialization attempt
 * \param serviceId the identifier of the service that failed to initialize
 * \param serviceInstanceId the instance identifier of the service that failed
 * \param sourceCluster the cluster from which the initialization failure originated
 */
void logInitFailure(
    LogLevel level,
    Error error,
    core::HRESULT res,
    uint16_t serviceId,
    uint16_t serviceInstanceId,
    uint8_t sourceCluster);

/**
 * Logs a failure encountered while sending a message.
 * Records a message sending failure with the specified severity level, error code,
 * result, and message details. This is typically called when message transmission fails due to
 * queue full, service not found, or other transmission errors.
 *
 * \param level the severity level of the log message
 * \param error the specific error that occurred during message sending
 * \param res the HRESULT indicating the result of the sending attempt
 * \param msg constant reference to the message object that failed to send
 */
void logMessageSendingFailure(
    LogLevel level, Error error, core::HRESULT res, core::Message const& msg);

/**
 * Logs a violation of cross-thread operations.
 * Records a cross-thread access violation with the specified severity level, error code,
 * and detailed information about the violation. This is called when a service or proxy is accessed
 * from a different thread than the one it was initialized on, which may violate thread-safety
 * requirements.
 *
 * \param level the severity level of the log message
 * \param error the specific error indicating the type of violation
 * \param sourceCluster the cluster from which the violation originated
 * \param serviceId the identifier of the service involved in the violation
 * \param serviceInstanceId the instance identifier of the service involved
 * \param initId the ID of the initialization task/thread
 * \param currentTaskId the ID of the current task/thread that caused the violation
 */
void logCrossThreadViolation(
    LogLevel level,
    Error error,
    uint8_t sourceCluster,
    uint16_t serviceId,
    uint16_t serviceInstanceId,
    uint32_t initId,
    uint32_t currentTaskId);

} // namespace logger
} // namespace middleware
