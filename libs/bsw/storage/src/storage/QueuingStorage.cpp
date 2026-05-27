/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

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
    auto& job = _jobs.front();
    _jobs.pop_front();
    lock.unlock();
    _storage.process(job);
    ::async::execute(_context, *this);
}

} // namespace storage
