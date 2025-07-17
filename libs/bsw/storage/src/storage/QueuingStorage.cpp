// Copyright 2025 Accenture.

#include <async/Types.h>
#include <storage/QueuingStorage.h>

namespace storage
{

QueuingStorage::QueuingStorage(IStorage& storage, ::async::ContextType const context)
: _storage(storage), _context(context)
{}

void QueuingStorage::process(StorageJob& job)
{
    ::async::ModifiableLockType lock;
    _jobs.push_back(job);
    lock.unlock();
    ::async::execute(_context, *this);
}

void QueuingStorage::execute()
{
    ::async::ModifiableLockType lock;
    if (_jobs.empty())
    {
        return;
    }
    StorageJob& job = _jobs.front();
    _jobs.pop_front();
    lock.unlock();
    _storage.process(job);
    ::async::execute(_context, *this);
}

} // namespace storage
