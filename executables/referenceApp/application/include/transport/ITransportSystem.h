// Copyright 2024 Accenture.

#ifndef GUARD_861EA210_3754_4464_8FC8_0CF9EE2CF8F7
#define GUARD_861EA210_3754_4464_8FC8_0CF9EE2CF8F7

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

#endif // GUARD_861EA210_3754_4464_8FC8_0CF9EE2CF8F7
