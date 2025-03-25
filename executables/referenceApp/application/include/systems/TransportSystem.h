// Copyright 2024 Accenture.

#pragma once

#include <lifecycle/AsyncLifecycleComponent.h>
#include <transport/ITransportMessageProvider.h>
#include <transport/ITransportSystem.h>
#include <transport/routing/TransportRouterSimple.h>

#include <estd/singleton.h>

namespace transport
{
class AbstractTransportLayer;

class TransportSystem
: public ::estd::singleton<TransportSystem>
, public ::transport::ITransportSystem
, public ::lifecycle::AsyncLifecycleComponent
{
public:
    explicit TransportSystem(::async::ContextType transitionContext);

    virtual char const* getName() const;

    void init() override;
    void run() override;
    void shutdown() override;

    virtual void dump() const;

    TransportRouterSimple& getTransportRouterSimple() { return _transportRouter; }

    /** \see ITransportSystem::addTransportLayer() */
    void addTransportLayer(AbstractTransportLayer& layer) override;
    /** \see ITransportSystem::removeTransportLayer() */
    void removeTransportLayer(AbstractTransportLayer& layer) override;
    /** \see ITransportSystem::getTransportMessageProvider() */
    ITransportMessageProvider& getTransportMessageProvider() override;

private:
    TransportRouterSimple _transportRouter;
};

} // namespace transport
