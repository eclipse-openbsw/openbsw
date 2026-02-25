// Copyright 2025 BMW AG

#pragma once

#include <etl/limits.h>
#include <etl/platform.h>

#include <cstdint>

namespace middleware
{
namespace core
{

ETL_INLINE_VAR constexpr uint16_t MAX_METHOD_ID       = 128U;
ETL_INLINE_VAR constexpr uint8_t INVALID_CLUSTER_ID   = etl::numeric_limits<uint8_t>::max();
ETL_INLINE_VAR constexpr uint16_t INVALID_SERVICE_ID  = etl::numeric_limits<uint16_t>::max();
ETL_INLINE_VAR constexpr uint16_t INVALID_INSTANCE_ID = etl::numeric_limits<uint16_t>::max();
ETL_INLINE_VAR constexpr uint16_t INVALID_REQUEST_ID  = etl::numeric_limits<uint16_t>::max();
ETL_INLINE_VAR constexpr uint8_t INVALID_ADDRESS_ID   = etl::numeric_limits<uint8_t>::max();
ETL_INLINE_VAR constexpr uint16_t FUTURE_DISPATCHER_OFFSET
    = static_cast<uint16_t>(sizeof(uint32_t) * 8U);
ETL_INLINE_VAR constexpr uint32_t INVALID_TASK_ID   = etl::numeric_limits<uint32_t>::max();
ETL_INLINE_VAR constexpr uint16_t INVALID_MEMBER_ID = etl::numeric_limits<uint16_t>::max();

enum class ErrorState : uint8_t
{
    NoError              = 0U,
    UserDefinedError     = 4U, //!< Must match IFuture::State::State_User_Error
    ServiceBusy          = 5U, //!< Must match IFuture::State::State_Service_Busy
    ServiceNotFound      = 6U, //!< Must match IFuture::State::State_Service_Not_Found
    SerializationError   = 7U, //!< Must match IFuture::State::State_Serialization_Error
    DeserializationError = 8U, //!< Must match IFuture::State::State_Deserialization_Error
    QueueFullError       = 9U  //!< Must match IFuture::State::State_CouldNotDeliver_Error
};

enum class HRESULT : uint8_t
{
    QueueFull                                   = 0xFFU,
    ServiceIdOutOfRange                         = 0xFDU,
    InstanceAlreadyRegistered                   = 0xFCU,
    InstanceNotFound                            = 0xFBU,
    NotRegistered                               = 0xFAU,
    ClusterIdNotFoundOrTransceiverNotRegistered = 0xF9U,
    RequestPoolDepleted                         = 0xF8U,
    NotImplemented                              = 0xF7U,
    WrongTargetClusterId                        = 0xF6U,
    CannotAllocatePayload                       = 0xF5U,
    CannotDeallocatePayload                     = 0xF4U,
    TransceiverInitializationFailed             = 0xF3U,
    UnknownMessageType                          = 0xF2U,
    ServiceNotFound                             = 0xF1U,
    FutureAlreadyInUse                          = 0xF0U,
    FutureNotFound                              = 0xEEU,
    SkeletonWithThisServiceIdAlreadyRegistered  = 0xEDU,
    ResponseBufferFutureNotFound                = 0xECU,
    NoClientsAvailable                          = 0xEBU,
    ServiceBusy                                 = 0xEAU,
    ServiceMemberIdNotFound                     = 0xE9U,
    EventNotSendSuccessfully                    = 0xE8U,
    RoutingError                                = 0xE7U,
    InvalidPayload                              = 0xE6U,
    InvalidRecipientCluster                     = 0xE5U,
    UnchangedValueNotSent                       = 0xE4U,
    DebouncedValueNotSent                       = 0xE3U,
    TimingValueNotSent                          = 0xE2U,
    Ok                                          = 0x00U
};

} // namespace core
} // namespace middleware
