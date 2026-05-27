/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

/**
 * \ingroup doip
 */
#pragma once

#include <etl/intrusive_links.h>
#include <transport/TransportMessage.h>
#include <platform/estdint.h>

namespace transport
{
class ITransportMessageProcessedListener;
}

namespace doip
{
/**
 * DoIP structure for holding a transport message reference together with some admin data.
 */
class DoIpTransportMessageRef : public ::etl::forward_link<0>
{
public:
    /**
     * Constructor.
     * \param message transport message to send
     * \param notificationListener optional listener to be notified when message has been sent
     * \param refCount initial value of ref counter
     */
    DoIpTransportMessageRef(
        ::transport::TransportMessage& message,
        ::transport::ITransportMessageProcessedListener* notificationListener,
        size_t refCount);

    /**
     * Get the represented transport message.
     * \return reference to the message
     */
    ::transport::TransportMessage& getMessage();
    /**
     * Get the optional notification listener.
     * \return pointer to notification listener, may be nullptr
     */
    ::transport::ITransportMessageProcessedListener* getNotificationListener() const;

    /**
     * Set a reference i.e. increase the reference counter
     */
    void setRef();

    /**
     * Get current reference count.
     */
    size_t getRef() const;

    /**
     * Release a reference.
     * \return new value of reference counter after decreasing
     */
    size_t releaseRef();

private:
    ::transport::TransportMessage& _message;
    ::transport::ITransportMessageProcessedListener* _notificationListener;
    size_t _refCount;
};

/**
 * Inline implementations.
 */
inline DoIpTransportMessageRef::DoIpTransportMessageRef(
    ::transport::TransportMessage& message,
    ::transport::ITransportMessageProcessedListener* const notificationListener,
    size_t const refCount)
: _message(message), _notificationListener(notificationListener), _refCount(refCount)
{}

inline ::transport::TransportMessage& DoIpTransportMessageRef::getMessage() { return _message; }

inline ::transport::ITransportMessageProcessedListener*
DoIpTransportMessageRef::getNotificationListener() const
{
    return _notificationListener;
}

inline void DoIpTransportMessageRef::setRef() { ++_refCount; }

inline size_t DoIpTransportMessageRef::getRef() const { return _refCount; }

inline size_t DoIpTransportMessageRef::releaseRef()
{
    --_refCount;
    return _refCount;
}

} // namespace doip
