// Copyright 2024 Accenture.

#ifndef GUARD_B9E6F2FF_EEDD_5CB9_EAF7_AAD589358F6C
#define GUARD_B9E6F2FF_EEDD_5CB9_EAF7_AAD589358F6C

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

#endif // GUARD_B9E6F2FF_EEDD_5CB9_EAF7_AAD589358F6C
