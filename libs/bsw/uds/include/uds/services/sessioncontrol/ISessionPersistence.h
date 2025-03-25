// Copyright 2024 Accenture.

#pragma once

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

