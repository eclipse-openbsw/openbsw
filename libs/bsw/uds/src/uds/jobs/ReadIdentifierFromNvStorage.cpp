// Copyright 2024 Accenture.

#include "uds/jobs/ReadIdentifierFromNvStorage.h"

#include "uds/connection/IncomingDiagConnection.h"

#include <etl/memory.h>
#include <etl/span.h>
#include <etl/unaligned_type.h>

namespace uds
{
ReadIdentifierFromNvStorage::ReadIdentifierFromNvStorage(
    IAsyncDiagHelper& asyncHelper,
    uint16_t const identifier,
    ::eeprom::IEepromHelper& iEepromHelper,
    ::nvstorage::NvBlockIdType const nvItem,
    size_t const length,
    DiagSessionMask const sessionMask)
: DataIdentifierJob(_implementedRequest, sessionMask)
, _asyncJobHelper(asyncHelper, *this)
, _ieepromHelper(iEepromHelper)
, _nvLengthItem(0)
, _nvItem(nvItem)
, _length(length)
, _lengthBuffer()
, _isVariableLength(false)
, _posResponse(nullptr)
, _pDiagConnection(nullptr)
, _nvReadJobfinished(::eeprom::EepromJobFinishedNotification::create<
                     ReadIdentifierFromNvStorage,
                     &ReadIdentifierFromNvStorage::onNvReadJobFinished>(*this))
, _nvLengthReadJobfinished(::eeprom::EepromJobFinishedNotification::create<
                           ReadIdentifierFromNvStorage,
                           &ReadIdentifierFromNvStorage::onLengthNvReadJobFinished>(*this))
{
    _implementedRequest[0] = 0x22U;
    _implementedRequest[1] = static_cast<uint8_t>((identifier >> 8) & 0xFFU);
    _implementedRequest[2] = static_cast<uint8_t>(identifier & 0xFFU);
}

ReadIdentifierFromNvStorage::ReadIdentifierFromNvStorage(
    IAsyncDiagHelper& asyncHelper,
    uint16_t const identifier,
    ::eeprom::IEepromHelper& iEepromHelper,
    nvstorage::NvBlockIdType const nvLengthItem,
    nvstorage::NvBlockIdType const nvItem,
    DiagSessionMask const sessionMask)
: DataIdentifierJob(_implementedRequest, sessionMask)
, _asyncJobHelper(asyncHelper, *this)
, _ieepromHelper(iEepromHelper)
, _nvLengthItem(nvLengthItem)
, _nvItem(nvItem)
, _length(0U)
, _lengthBuffer()
, _isVariableLength(true)
, _posResponse(nullptr)
, _pDiagConnection(nullptr)
, _nvReadJobfinished(::eeprom::EepromJobFinishedNotification::create<
                     ReadIdentifierFromNvStorage,
                     &ReadIdentifierFromNvStorage::onNvReadJobFinished>(*this))
, _nvLengthReadJobfinished(::eeprom::EepromJobFinishedNotification::create<
                           ReadIdentifierFromNvStorage,
                           &ReadIdentifierFromNvStorage::onLengthNvReadJobFinished>(*this))
{
    _implementedRequest[0] = 0x22U;
    _implementedRequest[1] = static_cast<uint8_t>((identifier >> 8) & 0xFFU);
    _implementedRequest[2] = static_cast<uint8_t>(identifier & 0xFFU);
}

DiagReturnCode::Type ReadIdentifierFromNvStorage::process(
    IncomingDiagConnection& connection, uint8_t const* const request, uint16_t const requestLength)
{
    if (_asyncJobHelper.hasPendingAsyncRequest())
    {
        return _asyncJobHelper.enqueueRequest(connection, request, requestLength);
    }
    _asyncJobHelper.startAsyncRequest(connection);

    _pDiagConnection = &connection;
    _posResponse     = &connection.releaseRequestGetResponse();

    if (_isVariableLength)
    {
        _length = 0U;
        if (!_ieepromHelper.read(
                _nvLengthItem, &_lengthBuffer[0U], sizeof(_lengthBuffer), _nvLengthReadJobfinished))
        {
            _posResponse     = nullptr;
            _pDiagConnection = nullptr;
            return DiagReturnCode::ISO_CONDITIONS_NOT_CORRECT;
        }
    }
    else
    {
        if (_posResponse->getMaximumLength() < _length)
        {
            _posResponse     = nullptr;
            _pDiagConnection = nullptr;
            return DiagReturnCode::ISO_CONDITIONS_NOT_CORRECT;
        }

        if (!_ieepromHelper.read(_nvItem, _posResponse->getData(), _length, _nvReadJobfinished))
        {
            _posResponse     = nullptr;
            _pDiagConnection = nullptr;
            return DiagReturnCode::ISO_CONDITIONS_NOT_CORRECT;
        }
    }

    return DiagReturnCode::OK;
}

void ReadIdentifierFromNvStorage::onLengthNvReadJobFinished(
    ::nvstorage::NvStorageOperation const op,
    ::nvstorage::NvStorageReturnCode const rc,
    ::nvstorage::NvBlockIdType const id)
{
    DiagReturnCode::Type nrc = DiagReturnCode::OK;

    if (nullptr == _posResponse)
    {
        return;
    }

    if (rc != ::nvstorage::NVSTORAGE_REQ_OK)
    {
        onNvReadJobFinished(op, rc, id);
        return;
    }

    _length = ::etl::be_uint32_t(_lengthBuffer);

    if (_posResponse->getMaximumLength() < _length)
    {
        nrc = DiagReturnCode::ISO_CONDITIONS_NOT_CORRECT;
    }

    if (!_ieepromHelper.read(_nvItem, _posResponse->getData(), _length, _nvReadJobfinished))
    {
        nrc = DiagReturnCode::ISO_CONDITIONS_NOT_CORRECT;
    }

    if (nrc != DiagReturnCode::OK)
    {
        _posResponse = nullptr;
        (void)getAndResetConnection(_pDiagConnection)->sendNegativeResponse(nrc, *this);
    }
}

void ReadIdentifierFromNvStorage::onNvReadJobFinished(
    ::nvstorage::NvStorageOperation const op,
    ::nvstorage::NvStorageReturnCode const rc,
    ::nvstorage::NvBlockIdType const id)
{
    if (nullptr == _posResponse)
    {
        return;
    }

    if (((rc != ::nvstorage::NVSTORAGE_REQ_OK)
         && (rc != ::nvstorage::NVSTORAGE_REQ_INTEGRITY_FAILED)
         && (rc != ::nvstorage::NVSTORAGE_REQ_RESTORED_FROM_ROM))
        || (op != ::nvstorage::NvStorageOperation::NVSTORAGE_READ) || (id != _nvItem))
    {
        _posResponse = nullptr;
        (void)getAndResetConnection(_pDiagConnection)
            ->sendNegativeResponse(DiagReturnCode::ISO_GENERAL_PROGRAMMING_FAILURE, *this);
    }
    else
    {
        if (rc == ::nvstorage::NVSTORAGE_REQ_INTEGRITY_FAILED)
        {
            ::etl::mem_set(_posResponse->getData(), _length, 0U);
        }
        PositiveResponse* const response = _posResponse;
        auto const connection            = _pDiagConnection;
        _posResponse                     = nullptr;
        _pDiagConnection                 = nullptr;
        (void)response->increaseDataLength(static_cast<uint16_t>(_length));
        (void)connection->sendPositiveResponseInternal(response->getLength(), *this);
    }
}

void ReadIdentifierFromNvStorage::responseSent(
    IncomingDiagConnection& connection, ResponseSendResult result)
{
    AbstractDiagJob::responseSent(connection, result);
    _asyncJobHelper.endAsyncRequest();
}

} // namespace uds
