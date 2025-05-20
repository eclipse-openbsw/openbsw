// Copyright 2025 Accenture.

#pragma once

#include <async/util/Call.h>
#include <storage/IStorage.h>
#include <storage/StorageJob.h>

#include <estd/array.h>
#include <estd/slice.h>

#include <cstring>

namespace storage
{

struct MappingConfig
{
    uint32_t const blockId;
    uint32_t const outgoingBlockId;
    uint8_t const outgoingIdx;
};

class MappingStorage
: public IStorage
, private ::async::RunnableType
{
public:
    ~MappingStorage()                                = default;
    MappingStorage(MappingStorage const&)            = delete;
    MappingStorage& operator=(MappingStorage const&) = delete;

    void process(StorageJob& job) final;

protected:
    explicit MappingStorage(
        MappingConfig const* blockConfig,
        size_t numBlocks,
        ::async::ContextType context,
        ::estd::slice<StorageJob*> inJobs,
        ::estd::slice<StorageJob*> outJobs,
        ::estd::slice<IStorage* const> storages);

private:
    void jobFailed(StorageJob& job);
    void execute() final;
    void triggerJob(StorageJob& jobIn, MappingConfig const& conf, size_t slotIdx);
    void callback(StorageJob& job);
    MappingConfig const* getBlockConfig(uint32_t blockId) const;
    size_t findUsedSlotIdx(StorageJob& job) const;

    MappingConfig const* const _blockConfig;
    size_t const _numBlocks;
    ::async::ContextType _context;
    ::estd::slice<StorageJob*> const _inJobs;
    ::estd::slice<StorageJob*> _outJobs;
    ::estd::slice<IStorage* const> const _storages;
    StorageJob::JobDoneCallback const _callback;
    ::estd::forward_list<StorageJob> _waitingJobs;
    ::estd::forward_list<StorageJob> _failedJobs;
};

namespace declare
{
// CONFIG_SIZE: number of entries in the blockConfig
// NUM_STORAGES: number of outgoing storages that will be passed in the constructor
// NUM_JOB_SLOTS: number of slots for outgoing jobs
template<size_t CONFIG_SIZE, size_t NUM_STORAGES, size_t NUM_JOB_SLOTS>
class MappingStorage : public ::storage::MappingStorage
{
    static_assert(CONFIG_SIZE > 0U, "number of blocks must be higher than 0");
    static_assert(NUM_STORAGES > 0U, "number of storages must be higher than 0");
    static_assert(NUM_JOB_SLOTS > 0U, "number of slots must be higher than 0");

public:
    // NOTE: the order of "storages" arguments is crucial and must match the indices (outgoingIdx)
    // given in the blockConfig (i.e. the first storage argument corresponds to the outgoingIdx 0,
    // the second to outgoingIdx 1 and so on)
    template<typename... T>
    explicit MappingStorage(
        MappingConfig const (&blockConfig)[CONFIG_SIZE],
        ::async::ContextType context,
        T&&... storages)
    : ::storage::MappingStorage(
        reinterpret_cast<MappingConfig const*>(&blockConfig),
        CONFIG_SIZE,
        context,
        _inJobs,
        _outJobPtrs,
        _storages)
    , _storages({&storages...})
    {
        static_assert(
            (sizeof...(storages)) == NUM_STORAGES,
            "number of storage arguments needs to be equal to NUM_STORAGES");
        (void)memset(_inJobs.data(), 0U, (_inJobs.size() * sizeof(void*)));
        for (size_t i = 0U; i < NUM_JOB_SLOTS; ++i)
        {
            _outJobPtrs[i] = &(_outJobs[i]);
        }
    }

private:
    ::estd::array<IStorage* const, NUM_STORAGES> _storages;
    ::estd::array<StorageJob*, NUM_JOB_SLOTS> _inJobs;
    ::estd::array<StorageJob, NUM_JOB_SLOTS> _outJobs;
    ::estd::array<StorageJob*, NUM_JOB_SLOTS> _outJobPtrs;
};
} // namespace declare

} // namespace storage
