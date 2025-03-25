// Copyright 2024 Accenture.

#pragma once

#include "util/stream/ISharedOutputStream.h"

namespace util
{
namespace stream
{
template<class Lock>
class SharedOutputStreamResource : public ISharedOutputStream
{
public:
    SharedOutputStreamResource(ISharedOutputStream& strm, Lock& lock);

    IOutputStream& startOutput(IContinuousUser* user) override;
    void endOutput(IContinuousUser* user) override;
    void releaseContinuousUser(IContinuousUser& user) override;

private:
    ISharedOutputStream& _stream;
    Lock& _lock;
    IContinuousUser* _user;
};

/**
 * Implementation.
 */
template<class Lock>
SharedOutputStreamResource<Lock>::SharedOutputStreamResource(ISharedOutputStream& strm, Lock& lock)
: _stream(strm), _lock(lock), _user(nullptr)
{}

template<class Lock>
IOutputStream& SharedOutputStreamResource<Lock>::startOutput(IContinuousUser* const user)
{
    _lock.lock();
    return _stream.startOutput(user);
}

template<class Lock>
void SharedOutputStreamResource<Lock>::endOutput(IContinuousUser* const user)
{
    _stream.endOutput(user);
    _lock.unlock();
}

template<class Lock>
void SharedOutputStreamResource<Lock>::releaseContinuousUser(IContinuousUser& user)
{
    _lock.lock();
    _stream.releaseContinuousUser(user);
    _lock.unlock();
}

} // namespace stream
} // namespace util
