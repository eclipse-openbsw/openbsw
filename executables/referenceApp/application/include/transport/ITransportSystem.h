/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "transport/ITransportMessageProvider.h"

namespace transport
{
class AbstractTransportLayer;

class ITransportSystem
{
public:
    /** Add a transport layer as a routing target */
    virtual void addTransportLayer(AbstractTransportLayer& layer)    = 0;
    /** Remove a transport layer as a routing target */
    virtual void removeTransportLayer(AbstractTransportLayer& layer) = 0;

    virtual ITransportMessageProvider& getTransportMessageProvider() = 0;
};

} // namespace transport
