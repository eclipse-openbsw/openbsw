// Copyright 2024 Accenture.

#ifndef GUARD_E4C956F8_4DDE_4F80_9C82_BBECDD4C15E5
#define GUARD_E4C956F8_4DDE_4F80_9C82_BBECDD4C15E5

#include "platform/estdint.h"

namespace uds
{
class DiagnosticSessionControl;

class ISessionPersistence
{
public:
    virtual void readSession(DiagnosticSessionControl& sessionControl) = 0;

    virtual void writeSession(DiagnosticSessionControl& sessionControl, uint8_t session) = 0;
};

} // namespace uds

#endif // GUARD_E4C956F8_4DDE_4F80_9C82_BBECDD4C15E5
