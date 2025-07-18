// Copyright 2024 Accenture.

#include "uds/jobs/WriteIdentifierToNvStorage.h"

#include "uds/connection/IncomingDiagConnection.h"

#include <etl/unaligned_type.h>

namespace uds
{
WriteIdentifierToNvStorage::WriteIdentifierToNvStorage(
    IAsyncDiagHelper& asyncHelper,
    uint16_t const identifier,
    ::eeprom::IEepromHelper& iEepromHelper,
    ::nvstorage::NvBlockIdType const nvItem,
    size_t const length,
    DiagSessionMask const sessionMask)
: AbstractDiagJob(
    _implementedRequest,
    static_cast<uint8_t>(sizeof(_implementedRequest)),
    1U,
    VARIABLE_REQUEST_LENGTH,
    VARIABLE_RESPONSE_LENGTH,
    sessionMask)
, _asyncJobHelper(asyncHelper, *this)
, _ieepromHelper(iEepromHelper)
, _nvLengthItem(0)
, _nvItem(nvItem)
, _length(length)
, _lengthBuff()
, _variableLength(false)
, _maxLength(0U)
, _pDiagConnection(nullptr)
, _pRequestData(nullptr)
, _nvWriteJobfinished(::eeprom::EepromJobFinishedNotification::create<
                      WriteIdentifierToNvStorage,
                      &WriteIdentifierToNvStorage::onNvWriteJobFinished>(*this))
, _nvLengthWriteJobfinished(::eeprom::EepromJobFinishedNotification::create<
                            WriteIdentifierToNvStorage,
                            &WriteIdentifierToNvStorage::onLengthNvWriteJobFinished>(*this))
{
    _implementedRequest[0] = 0x2EU;
    _implementedRequest[1] = static_cast<uint8_t>((identifier >> 8) & 0xFFU);
    _implementedRequest[2] = static_cast<uint8_t>(identifier & 0xFFU);
}

WriteIdentifierToNvStorage::WriteIdentifierToNvStorage(
    IAsyncDiagHelper& asyncHelper,
    uint16_t const identifier,
    ::eeprom::IEepromHelper& iEepromHelper,
    ::nvstorage::NvBlockIdType const nvLengthItem,
    ::nvstorage::NvBlockIdType const nvItem,
    size_t const maxLength,
    DiagSessionMask const sessionMask)
: AbstractDiagJob(
    _implementedRequest,
    static_cast<uint8_t>(sizeof(_implementedRequest)),
    1U,
    VARIABLE_REQUEST_LENGTH,
    VARIABLE_RESPONSE_LENGTH,
    sessionMask)
, _asyncJobHelper(asyncHelper, *this)
, _ieepromHelper(iEepromHelper)
, _nvLengthItem(nvLengthItem)
, _nvItem(nvItem)
, _length(0U)
, _lengthBuff()
, _variableLength(true)
, _maxLength(maxLength)
, _pDiagConnection(nullptr)
, _pRequestData(nullptr)
, _nvWriteJobfinished(::eeprom::EepromJobFinishedNotification::create<
                      WriteIdentifierToNvStorage,
                      &WriteIdentifierToNvStorage::onNvWriteJobFinished>(*this))
, _nvLengthWriteJobfinished(::eeprom::EepromJobFinishedNotification::create<
                            WriteIdentifierToNvStorage,
                            &WriteIdentifierToNvStorage::onLengthNvWriteJobFinished>(*this))
{
    _implementedRequest[0] = 0x2EU;
    _implementedRequest[1] = static_cast<uint8_t>((identifier >> 8) & 0xFFU);
    _implementedRequest[2] = static_cast<uint8_t>(identifier & 0xFFU);
}

DiagReturnCode::Type
WriteIdentifierToNvStorage::verify(uint8_t const* const request, uint16_t const requestLength)
{
    if (!compare(request, getImplementedRequest() + 1U, 2U))
    {
        return DiagReturnCode::NOT_RESPONSIBLE;
    }

    if (_variableLength)
    {
        if (requestLength > (_maxLength + 2))
        {
            return DiagReturnCode::ISO_INVALID_FORMAT;
        }
    }
    else
    {
        if (requestLength != (_length + 2))
        {
            return DiagReturnCode::ISO_INVALID_FORMAT;
        }
    }

    return DiagReturnCode::OK;
}

DiagReturnCode::Type WriteIdentifierToNvStorage::process(
    IncomingDiagConnection& connection, uint8_t const* const request, uint16_t const requestLength)
{
    if (_asyncJobHelper.hasPendingAsyncRequest())
    {
        return _asyncJobHelper.enqueueRequest(connection, request, requestLength);
    }
    _asyncJobHelper.startAsyncRequest(connection);

    _pDiagConnection = &connection;

    if (_variableLength)
    {
        _length                             = requestLength;
        _pRequestData                       = request;
        ::etl::be_uint32_ext_t{_lengthBuff} = requestLength;
        if (!_ieepromHelper.write(
                _nvLengthItem, &_lengthBuff[0U], sizeof(_lengthBuff), _nvLengthWriteJobfinished))
        {
            _pDiagConnection = nullptr;
            _pRequestData    = nullptr;
            return DiagReturnCode::ISO_CONDITIONS_NOT_CORRECT;
        }
    }
    else
    {
        if (!_ieepromHelper.write(_nvItem, request, _length, _nvWriteJobfinished))
        {
            _pDiagConnection = nullptr;
            return DiagReturnCode::ISO_CONDITIONS_NOT_CORRECT;
        }
    }

    return DiagReturnCode::OK;
}

void WriteIdentifierToNvStorage::onLengthNvWriteJobFinished(
    ::nvstorage::NvStorageOperation const op,
    ::nvstorage::NvStorageReturnCode const rc,
    ::nvstorage::NvBlockIdType const id)
{
    if (nullptr == _pDiagConnection)
    {
        return;
    }

    if ((rc != ::nvstorage::NVSTORAGE_REQ_OK)
        || (op != ::nvstorage::NvStorageOperation::NVSTORAGE_WRITE) || (id != _nvItem))
    {
        onNvWriteJobFinished(op, rc, id);
    }
    else
    {
        if (!_ieepromHelper.write(_nvItem, _pRequestData, _length, _nvWriteJobfinished))
        {
            _pRequestData = nullptr;
            (void)getAndResetConnection(_pDiagConnection)
                ->sendNegativeResponse(DiagReturnCode::ISO_GENERAL_PROGRAMMING_FAILURE, *this);
        }
    }
}

void WriteIdentifierToNvStorage::onNvWriteJobFinished(
    ::nvstorage::NvStorageOperation const op,
    ::nvstorage::NvStorageReturnCode const rc,
    ::nvstorage::NvBlockIdType const id)
{
    if (nullptr == _pDiagConnection)
    {
        return;
    }

    _pRequestData = nullptr;

    if ((rc != ::nvstorage::NVSTORAGE_REQ_OK)
        || (op != ::nvstorage::NvStorageOperation::NVSTORAGE_WRITE) || (id != _nvItem))
    {
        (void)getAndResetConnection(_pDiagConnection)
            ->sendNegativeResponse(DiagReturnCode::ISO_GENERAL_PROGRAMMING_FAILURE, *this);
    }
    else
    {
        PositiveResponse& response = _pDiagConnection->releaseRequestGetResponse();
        auto const connection      = _pDiagConnection;
        _pDiagConnection           = nullptr;
        (void)connection->sendPositiveResponseInternal(response.getLength(), *this);
    }
}

void WriteIdentifierToNvStorage::responseSent(
    IncomingDiagConnection& connection, ResponseSendResult result)
{
    AbstractDiagJob::responseSent(connection, result);
    _asyncJobHelper.endAsyncRequest();
}

} // namespace uds
