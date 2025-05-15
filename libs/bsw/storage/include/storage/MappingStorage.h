// Copyright 2025 Accenture.

#pragma once

#include <async/util/Call.h>
#include <etl/array.h>
#include <etl/intrusive_list.h>
#include <etl/span.h>
#include <storage/IStorage.h>
#include <storage/StorageJob.h>

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
        MappingConfig const* config,
        size_t configSize,
        ::async::ContextType context,
        ::etl::span<StorageJob*> inJobs,
        ::etl::span<StorageJob*> outJobs,
        ::etl::span<IStorage* const> storages);

private:
    void execute() final;
    void jobFailed(StorageJob& job);
    void triggerJob(StorageJob& jobIn, MappingConfig const& conf, size_t slotIdx);
    void callback(StorageJob& job);
    MappingConfig const* getConfigEntry(uint32_t blockId) const;
    size_t getUsedSlotIdx(StorageJob& job) const;

    MappingConfig const* const _config;
    size_t const _configSize;
    ::async::ContextType const _context;
    ::etl::span<StorageJob*> const _inJobs;
    ::etl::span<StorageJob*> const _outJobs;
    ::etl::span<IStorage* const> const _storages;
    StorageJob::JobDoneCallback const _callback;
    ::etl::intrusive_list<StorageJob, ::etl::bidirectional_link<0>> _waitingJobs;
    ::etl::intrusive_list<StorageJob, ::etl::bidirectional_link<0>> _failedJobs;
};

namespace declare
{
// CONFIG_SIZE: number of entries in the mapping config
// NUM_STORAGES: number of outgoing storages that will be passed in the constructor
// NUM_JOB_SLOTS: number of slots for outgoing jobs
template<size_t CONFIG_SIZE, size_t NUM_STORAGES, size_t NUM_JOB_SLOTS>
class MappingStorage : public ::storage::MappingStorage
{
    static_assert(CONFIG_SIZE > 0U, "number of blocks must be bigger than 0");
    static_assert(NUM_STORAGES > 0U, "number of storages must be bigger than 0");
    static_assert(NUM_JOB_SLOTS > 0U, "number of slots must be bigger than 0");

public:
    // NOTE: the order of "storages" arguments is crucial and must match the indices (outgoingIdx)
    // given in the config (i.e. the first storage argument corresponds to the outgoingIdx 0,
    // the second to outgoingIdx 1 and so on)
    template<typename... T>
    explicit MappingStorage(
        MappingConfig const (&config)[CONFIG_SIZE],
        ::async::ContextType const context,
        T&&... storages)
    : ::storage::MappingStorage(
        reinterpret_cast<MappingConfig const*>(&config),
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
        _inJobs.fill(nullptr);
        for (size_t i = 0U; i < NUM_JOB_SLOTS; ++i)
        {
            _outJobPtrs[i] = &(_outJobs[i]);
        }
    }

private:
    ::etl::array<IStorage* const, NUM_STORAGES> _storages;
    ::etl::array<StorageJob*, NUM_JOB_SLOTS> _inJobs;
    ::etl::array<StorageJob, NUM_JOB_SLOTS> _outJobs;
    ::etl::array<StorageJob*, NUM_JOB_SLOTS> _outJobPtrs;
};
} // namespace declare

} // namespace storage
