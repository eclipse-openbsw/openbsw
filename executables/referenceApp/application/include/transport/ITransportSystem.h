// Copyright 2024 Accenture.

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
