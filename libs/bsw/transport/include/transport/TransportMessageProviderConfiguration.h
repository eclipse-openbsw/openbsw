// Copyright 2024 Accenture.

/**
 * \ingroup     transport
 */
#ifndef GUARD_CE2171F1_2F35_48FB_8DE7_AFB49C45CA5D
#define GUARD_CE2171F1_2F35_48FB_8DE7_AFB49C45CA5D

#include "transport/BufferedTransportMessage.h"

#include <estd/object_pool.h>
#include <estd/uncopyable.h>

#include <cstdint>

namespace transport
{
class ITransportMessageProviderConfiguration
{
protected:
    ITransportMessageProviderConfiguration() = default;

public:
    ITransportMessageProviderConfiguration(ITransportMessageProviderConfiguration const&) = delete;
    ITransportMessageProviderConfiguration& operator=(ITransportMessageProviderConfiguration const&)
        = delete;

    /**
     * Returns the TransportMessage's underlying buffer's size.
     * \see TransportMessage::getBufferLength()
     */
    virtual uint32_t getBufferSize() const = 0;

    /**
     * Returns whether there is any TransportMessage left to acquire.
     */
    virtual bool empty() const = 0;

    /**
     * Acquires a new TransportMessage and returns a reference to it.
     * \assert{!empty()}
     */
    virtual TransportMessage& acquire() = 0;

    /**
     * Releases a TransportMessage to the underlying pool.
     */
    virtual void release(TransportMessage& msg) = 0;
};

namespace declare
{
template<uint8_t NUM_BUFFERS, uint32_t PAYLOAD_SIZE>
class TransportMessageProviderConfiguration
: public ::transport::ITransportMessageProviderConfiguration
{
public:
    TransportMessageProviderConfiguration();

    /** \see    ITransportMessageProviderConfiguration::getBufferSize() */
    uint32_t getBufferSize() const override;

    /** \see    ITransportMessageProviderConfiguration::empty() */
    bool empty() const override;

    /** \see    ITransportMessageProviderConfiguration::acquire() */
    TransportMessage& acquire() override;

    /** \see    ITransportMessageProviderConfiguration::release() */
    void release(TransportMessage& msg) override;

private:
    using TransportMessagePool
        = ::estd::declare::object_pool<BufferedTransportMessage<PAYLOAD_SIZE>, NUM_BUFFERS>;
    TransportMessagePool fTransportMessagesPool;
};
} // namespace declare

/*
 *
 * Implementation
 *
 */
namespace declare
{
template<uint8_t NUM_BUFFERS, uint32_t BUFFER_SIZE>
inline TransportMessageProviderConfiguration<NUM_BUFFERS, BUFFER_SIZE>::
    TransportMessageProviderConfiguration()
: ::transport::ITransportMessageProviderConfiguration(), fTransportMessagesPool()
{}

// virtual
template<uint8_t NUM_BUFFERS, uint32_t PAYLOAD_SIZE>
uint32_t TransportMessageProviderConfiguration<NUM_BUFFERS, PAYLOAD_SIZE>::getBufferSize() const
{
    return PAYLOAD_SIZE;
}

// virtual
template<uint8_t NUM_BUFFERS, uint32_t PAYLOAD_SIZE>
bool TransportMessageProviderConfiguration<NUM_BUFFERS, PAYLOAD_SIZE>::empty() const
{
    return fTransportMessagesPool.empty();
}

// virtual
template<uint8_t NUM_BUFFERS, uint32_t PAYLOAD_SIZE>
TransportMessage& TransportMessageProviderConfiguration<NUM_BUFFERS, PAYLOAD_SIZE>::acquire()
{
    return fTransportMessagesPool.acquire();
}

// virtual
template<uint8_t NUM_BUFFERS, uint32_t PAYLOAD_SIZE>
void TransportMessageProviderConfiguration<NUM_BUFFERS, PAYLOAD_SIZE>::release(
    TransportMessage& msg)
{
    // This static cast is actually save because BufferedTransportMessage has TransportMessage
    // as its base class. So the pointer will not change.
    // However, it relies on the fact, that an object_pool checks if the released object
    // belongs to it.
    fTransportMessagesPool.release(static_cast<BufferedTransportMessage<PAYLOAD_SIZE>&>(msg));
}
} // namespace declare
} // namespace transport

#endif /*GUARD_CE2171F1_2F35_48FB_8DE7_AFB49C45CA5D*/
