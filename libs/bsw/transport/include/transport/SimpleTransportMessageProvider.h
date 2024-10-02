// Copyright 2024 Accenture.

/**
 * \ingroup transport
 */
#ifndef GUARD_8B8F2471_2ED6_478A_BB3C_9C5EE19D39E5
#define GUARD_8B8F2471_2ED6_478A_BB3C_9C5EE19D39E5

#include "transport/ITransportMessageProvidingListener.h"
#include "transport/TransportMessageProviderConfiguration.h"

#include <estd/uncopyable.h>

namespace transport
{
class SimpleTransportMessageProvider : public ITransportMessageProvider
{
    UNCOPYABLE(SimpleTransportMessageProvider);

public:
    explicit SimpleTransportMessageProvider(ITransportMessageProviderConfiguration& configuration);

    /**
     * \see ITransportMessageProvidingListener::getTransportMessage()
     */
    ITransportMessageProvidingListener::ErrorCode getTransportMessage(
        uint8_t srcBusId,
        uint16_t sourceId,
        uint16_t targetId,
        uint16_t size,
        ::estd::slice<uint8_t const> const& peek,
        ::transport::TransportMessage*& pTransportMessage) override;

    /**
     * \see ITransportMessageProvidingListener::releaseTransportMessage()
     */
    void releaseTransportMessage(TransportMessage& transportMessage) override;

    /**
     * \see ITransportMessageProviding::dump()
     */
    void dump() override;

private:
    friend class TransportRoutingTableProvider;
    friend class TransportRouter;
    ITransportMessageProviderConfiguration& fConfiguration;
};

} // namespace transport

#endif /*GUARD_8B8F2471_2ED6_478A_BB3C_9C5EE19D39E5*/
