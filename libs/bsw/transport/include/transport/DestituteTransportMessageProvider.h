// Copyright 2024 Accenture.

/**
 * \ingroup transport
 */
#ifndef GUARD_ADDC89BF_1AB5_40EA_951C_534A971A383B
#define GUARD_ADDC89BF_1AB5_40EA_951C_534A971A383B

#include "transport/ITransportMessageProvidingListener.h"

namespace transport
{
/**
 * An implementation of ::transport::ITransportMessageProvider, which always returns
 * TPMSG_NOT_RESPONSIBLE to getTransportMessage().
 */
class DestituteTransportMessageProvider : public ITransportMessageProvider
{
public:
    DestituteTransportMessageProvider();

    /**
     * This function will always return TPMSG_NOT_RESPONSIBLE and set pTransportMessage to NULL.
     *
     * \see ITransportMessageProvidingListener::getTransportMessage()
     */
    ITransportMessageProvidingListener::ErrorCode getTransportMessage(
        uint8_t srcBusId,
        uint16_t sourceId,
        uint16_t targetId,
        uint16_t size,
        ::estd::slice<uint8_t const> const& peek,
        TransportMessage*& pTransportMessage) override;

    /**
     * \see ITransportMessageProvidingListener::releaseTransportMessage()
     */
    void releaseTransportMessage(TransportMessage& transportMessage) override;

    void dump() override;
};

} // namespace transport

#endif /*GUARD_ADDC89BF_1AB5_40EA_951C_534A971A383B*/
