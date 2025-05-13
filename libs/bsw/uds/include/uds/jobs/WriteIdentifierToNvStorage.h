// Copyright 2024 Accenture.

#pragma once

#include "platform/estdint.h"
#include "uds/async/AsyncDiagJobHelper.h"
#include "uds/base/AbstractDiagJob.h"
#include "util/eeprom/IEepromHelper.h"

#include <etl/uncopyable.h>

namespace uds
{
/**
 * Generic implementation of a WriteDataByIdentifier which can write the
 * received data to NvStorage
 */
class WriteIdentifierToNvStorage
: public AbstractDiagJob
, public ::etl::uncopyable
{
public:
    /**
     * \brief Fixed length functionality
     *
     * Using this constructor the fixed length functionality is enabled.
     * The received data is just stored to NVStorage as is.
     */
    WriteIdentifierToNvStorage(
        IAsyncDiagHelper& asyncHelper,
        uint16_t const identifier,
        ::eeprom::IEepromHelper& iEepromHelper,
        ::nvstorage::NvBlockIdType const nvItem,
        size_t const length,
        DiagSessionMask const sessionMask = DiagSession::ALL_SESSIONS());

    /**
     * \brief Variable length functionality
     *
     * Using this constructor the variable length functionality is enabled.
     * The length is stored in the first 4 bytes of the NV item as big-endian.
     *
     * \remark For this to work the NV item must be maxLength + 4 Bytes.
     */
    WriteIdentifierToNvStorage(
        IAsyncDiagHelper& asyncHelper,
        uint16_t const identifier,
        ::eeprom::IEepromHelper& iEepromHelper,
        ::nvstorage::NvBlockIdType const nvLengthItem,
        ::nvstorage::NvBlockIdType const nvItem,
        size_t const maxLength,
        DiagSessionMask const sessionMask = DiagSession::ALL_SESSIONS());

private:
    DiagReturnCode::Type verify(uint8_t const request[], uint16_t requestLength) override;

    DiagReturnCode::Type process(
        IncomingDiagConnection& connection,
        uint8_t const request[],
        uint16_t requestLength) override;

    void responseSent(IncomingDiagConnection& connection, ResponseSendResult result) override;

    void onLengthNvWriteJobFinished(
        ::nvstorage::NvStorageOperation const op,
        ::nvstorage::NvStorageReturnCode const rc,
        ::nvstorage::NvBlockIdType const id);

    void onNvWriteJobFinished(
        ::nvstorage::NvStorageOperation const op,
        ::nvstorage::NvStorageReturnCode const rc,
        ::nvstorage::NvBlockIdType const id);

    uint8_t _implementedRequest[3];
    AsyncDiagJobHelper _asyncJobHelper;
    ::eeprom::IEepromHelper& _ieepromHelper;
    ::nvstorage::NvBlockIdType const _nvLengthItem;
    ::nvstorage::NvBlockIdType const _nvItem;
    size_t _length;
    uint8_t _lengthBuff[sizeof(uint32_t)];
    bool const _variableLength;
    size_t const _maxLength;
    IncomingDiagConnection* _pDiagConnection;
    uint8_t const* _pRequestData;
    ::eeprom::EepromJobFinishedNotification _nvWriteJobfinished;
    ::eeprom::EepromJobFinishedNotification _nvLengthWriteJobfinished;
};

} // namespace uds
