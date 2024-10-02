// Copyright 2024 Accenture.

#include "uds/services/sessioncontrol/EepromManagerSessionPersistence.h"

// #include "eeprom/EepromTable.h"
#include "nvstorage/NvBlockConfig.h"
#include "uds/UdsLogger.h"
#include "uds/services/sessioncontrol/DiagnosticSessionControl.h"

namespace uds
{
using ::nvstorage::NvBlockConfig;
using ::util::logger::Logger;
using ::util::logger::UDS;

EepromManagerSessionPersistence::EepromManagerSessionPersistence(
    ::eeprom::IEepromHelper& iEepromHelper)
: fpDiagnosticSessionControlRead(nullptr)
, fpDiagnosticSessionControlWrite(nullptr)
, fSessionBuffer{0U}
, _ieepromHelper(iEepromHelper)
{}

void EepromManagerSessionPersistence::readSession(DiagnosticSessionControl& sessionControl)
{
    fpDiagnosticSessionControlRead = &sessionControl;
    auto const jobfinished         = ::eeprom::EepromJobFinishedNotification::
        create<EepromManagerSessionPersistence, &EepromManagerSessionPersistence::jobFinished>(
            *this);
    if (!_ieepromHelper.read(NvBlockConfig::BootConfig, &fSessionBuffer, 1U, jobfinished))
    {
        Logger::error(UDS, "EepromManagerSessionPersistence::read: error reading from eeprom");
        fSessionBuffer = 0xFFU;
        sessionControl.sessionRead(fSessionBuffer);
        return;
    }
}

void EepromManagerSessionPersistence::writeSession(
    DiagnosticSessionControl& sessionControl, uint8_t const session)
{
    fpDiagnosticSessionControlWrite = &sessionControl;
    fSessionBuffer                  = session;
    auto const jobfinished          = ::eeprom::EepromJobFinishedNotification::
        create<EepromManagerSessionPersistence, &EepromManagerSessionPersistence::jobFinished>(
            *this);

    if (!_ieepromHelper.write(NvBlockConfig::BootConfig, &fSessionBuffer, 1U, jobfinished))
    {
        Logger::error(UDS, "EepromManagerSessionPersistence::read: error writing to eeprom");
        sessionControl.sessionWritten(false);
        return;
    }
}

void EepromManagerSessionPersistence::jobFinished(
    ::nvstorage::NvStorageOperation const op,
    ::nvstorage::NvStorageReturnCode const res,
    ::nvstorage::NvBlockIdType const /*id*/)
{
    if (op == ::nvstorage::NVSTORAGE_READ)
    {
        fpDiagnosticSessionControlRead->sessionRead(fSessionBuffer);
        fpDiagnosticSessionControlRead = nullptr;
    }
    else if (op == ::nvstorage::NVSTORAGE_WRITE)
    {
        fpDiagnosticSessionControlWrite->sessionWritten(res == ::nvstorage::NVSTORAGE_REQ_OK);
        fpDiagnosticSessionControlWrite = nullptr;
    }
    else
    {
        // nothing to do
    }
}

} // namespace uds
