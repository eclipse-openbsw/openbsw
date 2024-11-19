// Copyright 2024 Accenture.

#ifndef GUARD_FACE7DDD_046B_4B7B_B83F_6212136CBA2E
#define GUARD_FACE7DDD_046B_4B7B_B83F_6212136CBA2E

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

#endif /*GUARD_FACE7DDD_046B_4B7B_B83F_6212136CBA2E*/
