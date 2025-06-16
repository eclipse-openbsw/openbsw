// Copyright 2025 Accenture.

#pragma once

// TODO: uses IEepromEmulationDriver interface, gets a config with table of block
// IDs and maximum allowed size, processes synchronously each job

#include <storage/IStorage.h>
#include <storage/StorageJob.h>

namespace storage
{

class FeeStorage : public IStorage
{
public:
    explicit FeeStorage() {}

    ~FeeStorage()                            = default;
    FeeStorage(FeeStorage const&)            = delete;
    FeeStorage& operator=(FeeStorage const&) = delete;

    void process(StorageJob& job) final
    {
        if (job.is<StorageJob::Type::Write>())
        {
            job.sendResult(StorageJob::Result::Success());
            return;
        }
        else if (job.is<StorageJob::Type::Read>())
        {
            auto& readJob                       = job.getRead();
            // return fake data for now
            readJob.getBuffer().getBuffer()[0U] = 222U;
            readJob.setReadSize(1U);
            job.sendResult(StorageJob::Result::Success());
            return;
        }
        else
        {
            job.sendResult(StorageJob::Result::Error());
        }
    }
};

} // namespace storage
