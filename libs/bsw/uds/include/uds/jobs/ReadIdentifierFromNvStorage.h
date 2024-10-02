// Copyright 2024 Accenture.

#ifndef GUARD_CD9511BA_E53E_4A4F_B67E_C77B9C04A598
#define GUARD_CD9511BA_E53E_4A4F_B67E_C77B9C04A598

#include "estd/uncopyable.h"
#include "platform/estdint.h"
#include "uds/async/AsyncDiagJobHelper.h"
#include "uds/connection/PositiveResponse.h"
#include "uds/jobs/DataIdentifierJob.h"
#include "util/eeprom/IEepromHelper.h"

namespace uds
{
/**
 * Generic implementation of a ReadDataByIdentifier which can respond with a
 * value read from NvStorage.
 */
class ReadIdentifierFromNvStorage : public DataIdentifierJob
{
    UNCOPYABLE(ReadIdentifierFromNvStorage);

public:
    /**
     * \brief Fixed length functionality
     *
     * Using this constructor the fixed length functionality is enabled. The NV-Item
     * is read with the given offset and length. The response uses the plain data.
     */
    ReadIdentifierFromNvStorage(
        IAsyncDiagHelper& asyncHelper,
        ::async::ContextType const diagContext,
        uint16_t const identifier,
        ::eeprom::IEepromHelper& iEepromHelper,
        ::nvstorage::NvBlockIdType const nvItem,
        size_t const length,
        DiagSessionMask const sessionMask = DiagSession::ALL_SESSIONS());

    /**
     * \brief Variable length functionality
     *
     * Using this constructor the variable length functionality is enabled.
     * Variable length block will be configured as 2 blocks in nvStorage
     * 1st block is a 4-bytes one, which contains the length of the other block
     *  The content of the first NV-Item is interpreted as the length in Big-Endian.
     */
    ReadIdentifierFromNvStorage(
        IAsyncDiagHelper& asyncHelper,
        ::async::ContextType const diagContext,
        uint16_t const identifier,
        ::eeprom::IEepromHelper& iEepromHelper,
        nvstorage::NvBlockIdType const nvLengthItem,
        nvstorage::NvBlockIdType const nvItem,
        DiagSessionMask const sessionMask = DiagSession::ALL_SESSIONS());

private:
    DiagReturnCode::Type process(
        IncomingDiagConnection& connection,
        uint8_t const request[],
        uint16_t requestLength) override;

    void onNvReadJobFinished(
        ::nvstorage::NvStorageOperation const op,
        ::nvstorage::NvStorageReturnCode const rc,
        ::nvstorage::NvBlockIdType const id);

    void onLengthNvReadJobFinished(
        ::nvstorage::NvStorageOperation const op,
        ::nvstorage::NvStorageReturnCode const rc,
        ::nvstorage::NvBlockIdType const id);

    void responseSent(IncomingDiagConnection& connection, ResponseSendResult result) override;

    uint8_t _implementedRequest[3];
    AsyncDiagJobHelper _asyncJobHelper;
    ::eeprom::IEepromHelper& _ieepromHelper;
    ::nvstorage::NvBlockIdType const _nvLengthItem;
    ::nvstorage::NvBlockIdType const _nvItem;
    size_t _length;
    uint8_t _lengthBuffer[sizeof(uint32_t)];
    bool const _isVariableLength;
    PositiveResponse* _posResponse;
    IncomingDiagConnection* _pDiagConnection;
    ::eeprom::EepromJobFinishedNotification _nvReadJobfinished;
    ::eeprom::EepromJobFinishedNotification _nvLengthReadJobfinished;
};

} // namespace uds

#endif // GUARD_CD9511BA_E53E_4A4F_B67E_C77B9C04A598
