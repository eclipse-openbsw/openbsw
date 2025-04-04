// Copyright 2024 Accenture.

#pragma once

#include <estd/functional.h>

#include <cstdint>

namespace nvstorage
{
/** Block ID datatype */
using NvBlockIdType = uint8_t;
using NvAppIdType   = uint8_t;

namespace consts
{
constexpr NvBlockIdType INVALID_NV_BLOCK_ID = 0xffU;
} // namespace consts

/** Operation kind */
enum NvStorageOperation
{
    NVSTORAGE_READ,
    NVSTORAGE_WRITE,
    NVSTORAGE_READ_DEFAULTS,
    NVSTORAGE_INVALIDATE,
    NVSTORAGE_RADDR,
    NVSTORAGE_SWRITE,
    NVSTORAGE_READ_ALL,
    NVSTORAGE_WRITE_ALL,
    NVSTORAGE_NONE
};

/** Internal return code for validity info */
enum NvValidity
{
    NV_VALIDITY_OK,
    NV_VALIDITY_INVALID,
    NV_VALIDITY_READ_FAILED
};

/** Job priority */
enum NvPriority
{
    NVSTORAGE_PRIORITY_HIGH,
    NVSTORAGE_PRIORITY_MEDIUM,
    NVSTORAGE_PRIORITY_LOW
};

/** Return code */
enum NvStorageReturnCode
{
    NVSTORAGE_REQ_OK,
    NVSTORAGE_REQ_NOT_OK,
    NVSTORAGE_REQ_PENDING,
    NVSTORAGE_REQ_INTEGRITY_FAILED,
    NVSTORAGE_REQ_RESTORED_FROM_ROM,
    NVSTORAGE_REQ_DRIVER_ERROR,
    NVSTORAGE_REQ_NO_DRIVER,
    NVSTORAGE_REQ_INVALID_PARAMETER,
    NVSTORAGE_REQ_INVALIDATED,
    NVSTORAGE_REQ_DECRYPTION_FAILED,
    NVSTORAGE_REQ_ENCRYPTION_FAILED,
    NVSTORAGE_REQ_NOT_INITIALIZED,
    NVSTORAGE_REQ_NO_MILEAGE,
    NVSTORAGE_REQ_NV_BUSY
};

using applJobFinishedCallback = ::estd::function<void(
    NvStorageOperation const, NvStorageReturnCode const, NvBlockIdType const)>;
using applInitBlockCallback = ::estd::function<void(NvStorageOperation const, NvBlockIdType const)>;
} /* namespace nvstorage */
