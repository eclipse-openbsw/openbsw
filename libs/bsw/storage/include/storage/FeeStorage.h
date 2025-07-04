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
        StorageJob::ResultType result = StorageJob::Result::Error();
        if (job.is<StorageJob::Type::Write>())
        {
            result = StorageJob::Result::Success();
        }
        else if (job.is<StorageJob::Type::Read>())
        {
            auto& readJob                       = job.getRead();
            // return fake data for now
            readJob.getBuffer().getBuffer()[0U] = 222U;
            readJob.setReadSize(1U);
            result = StorageJob::Result::Success();
        }
        job.sendResult(result);
    }
};

} // namespace storage
