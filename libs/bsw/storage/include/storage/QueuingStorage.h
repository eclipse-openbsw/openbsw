// Copyright 2025 Accenture.

#pragma once

#include <async/util/Call.h>
#include <storage/IStorage.h>
#include <storage/StorageJob.h>

#include <estd/forward_list.h>

namespace storage
{

class QueuingStorage
: public IStorage
, private ::async::RunnableType
{
public:
    explicit QueuingStorage(IStorage& storage, ::async::ContextType context);
    ~QueuingStorage()                                = default;
    QueuingStorage(QueuingStorage const&)            = delete;
    QueuingStorage& operator=(QueuingStorage const&) = delete;

    void process(StorageJob& job) final;

private:
    void execute() final;

    IStorage& _storage;
    ::async::ContextType _context;
    ::estd::forward_list<StorageJob> _jobs;
};

} // namespace storage
