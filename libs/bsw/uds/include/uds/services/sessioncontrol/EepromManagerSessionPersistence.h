// Copyright 2024 Accenture.

#ifndef GUARD_7EB5E5D9_1034_4B5D_BC01_0C18909432A6
#define GUARD_7EB5E5D9_1034_4B5D_BC01_0C18909432A6

#include "nvstorage/NvStorageTypes.h"
#include "platform/estdint.h"
#include "uds/services/sessioncontrol/ISessionPersistence.h"
#include "util/eeprom/IEepromHelper.h"

namespace uds
{
class DiagnosticSessionControl;

class EepromManagerSessionPersistence : public ISessionPersistence
{
public:
    EepromManagerSessionPersistence(::eeprom::IEepromHelper& iEepromHelper);

    void readSession(DiagnosticSessionControl& sessionControl) override;

    void writeSession(DiagnosticSessionControl& sessionControl, uint8_t session) override;

private:
    void jobFinished(
        ::nvstorage::NvStorageOperation const op,
        ::nvstorage::NvStorageReturnCode const res,
        ::nvstorage::NvBlockIdType const /*id*/);

    DiagnosticSessionControl* fpDiagnosticSessionControlRead;
    DiagnosticSessionControl* fpDiagnosticSessionControlWrite;
    uint8_t fSessionBuffer;
    ::eeprom::IEepromHelper& _ieepromHelper;
};

} // namespace uds

#endif // GUARD_7EB5E5D9_1034_4B5D_BC01_0C18909432A6
