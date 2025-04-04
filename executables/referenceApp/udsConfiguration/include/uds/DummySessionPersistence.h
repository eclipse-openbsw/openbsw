// Copyright 2024 Accenture.

#pragma once

#include "uds/services/sessioncontrol/ISessionPersistence.h"

namespace uds
{
class DummySessionPersistence : public ISessionPersistence
{
public:
    void readSession(DiagnosticSessionControl&) override {}

    void writeSession(DiagnosticSessionControl&, uint8_t) override {}
};

} // namespace uds
