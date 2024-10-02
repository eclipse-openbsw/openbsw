// Copyright 2024 Accenture.

#include "uds/application/AbstractDiagApplication.h"

namespace uds
{
AbstractDiagApplication::AbstractDiagApplication(
    IOutgoingDiagConnectionProvider& connectionProvider)
: fConnectionProvider(connectionProvider)
{}

} // namespace uds
