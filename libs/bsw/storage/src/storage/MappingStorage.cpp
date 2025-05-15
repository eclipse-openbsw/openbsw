// Copyright 2025 Accenture.

#include <async/Types.h>
#include <etl/algorithm.h>
#include <storage/MappingStorage.h>
#include <util/estd/assert.h>

namespace storage
{

MappingStorage::MappingStorage(
    MappingConfig const* const config,
    size_t const configSize,
    ::async::ContextType const context,
    ::etl::span<StorageJob*> const inJobs,
    ::etl::span<StorageJob*> const outJobs,
    ::etl::span<IStorage* const> const storages)
: _config(config)
, _configSize(configSize)
, _context(context)
, _inJobs(inJobs)
, _outJobs(outJobs)
, _storages(storages)
, _callback(StorageJob::JobDoneCallback::create<MappingStorage, &MappingStorage::callback>(*this))
{}

void MappingStorage::process(StorageJob& job)
{
    if (job.is<StorageJob::Type::None>())
    {
        // job not initialized as read or write
        jobFailed(job);
        return;
    }

    auto* const confEntry = getConfigEntry(job.getId());
    if (confEntry == nullptr)
    {
        // no entry found for this blockId
        jobFailed(job);
        return;
    }

    if (confEntry->outgoingIdx >= _storages.size())
    {
        // outgoingIdx out of bounds
        jobFailed(job);
        return;
    }

    {
        // lock needed because _inJobs and _waitingJobs can be accessed from multiple tasks
        ::async::ModifiableLockType lock;
        for (size_t slotIdx = 0U; slotIdx < _inJobs.size(); ++slotIdx)
        {
            if (_inJobs[slotIdx] == nullptr)
            {
                // store incoming job so we can look it up again in the callback
                _inJobs[slotIdx] = &job;
                lock.unlock(); // variables inside triggerJob don't need to be protected
                triggerJob(job, *confEntry, slotIdx);
                return;
            }
        }
        // busy, store job for later and return
        _waitingJobs.push_back(job);
    }
}

void MappingStorage::execute()
{
    ::async::ModifiableLockType lock;
    if (_failedJobs.empty())
    {
        return;
    }
    StorageJob& job = _failedJobs.front();
    _failedJobs.pop_front();
    lock.unlock();
    job.sendResult(StorageJob::Result::Error());
    ::async::execute(_context, *this);
}

void MappingStorage::jobFailed(StorageJob& job)
{
    // put failed jobs in a queue and handle them asynchronously instead of calling job.sendResult
    // directly, otherwise there's a risk of stack overflow in case user calls process() again
    // inside the callback
    ::async::ModifiableLockType lock;
    _failedJobs.push_back(job);
    lock.unlock();
    ::async::execute(_context, *this);
}

void MappingStorage::triggerJob(
    StorageJob& jobIn, MappingConfig const& confEntry, size_t const slotIdx)
{
    // use one of the temporary jobs as the outgoing job to be passed to the lower-level storage
    auto& jobOut = *(_outJobs[slotIdx]);
    jobOut.init(confEntry.outgoingBlockId, _callback);
    if (jobIn.is<StorageJob::Type::Read>())
    {
        auto& readJob = jobIn.getRead();
        jobOut.initRead(readJob.getBuffer(), readJob.getOffset());
    }
    else
    {
        auto& writeJob = jobIn.getWrite();
        jobOut.initWrite(writeJob.getBuffer(), writeJob.getOffset());
    }
    // delegate the job to the correct outgoing storage
    _storages[confEntry.outgoingIdx]->process(jobOut);
}

void MappingStorage::callback(StorageJob& job)
{
    auto const slotIdx = getUsedSlotIdx(job);
    // slotIdx will be out of bounds if job object is unknown
    estd_assert(slotIdx < _inJobs.size());
    auto* jobInPtr = _inJobs[slotIdx];
    // if jobInPtr is already null, a storage has probably run the callback too many times; this
    // problem is not detectable here though in case there were items in the waiting list during
    // the previous call and the outgoing job was re-used for the next job; in this case the
    // problem is less obvious and a wrong callback might get called instead, but let's just
    // crash here if anything seems wrong
    estd_assert(jobInPtr != nullptr);
    auto& jobIn = *jobInPtr;
    if (job.is<StorageJob::Type::Read>())
    {
        // report how many bytes were read
        jobIn.getRead().setReadSize(job.getRead().getReadSize());
    }
    // trigger the user-provided callback
    jobIn.sendResult(job.getResult());

    {
        ::async::ModifiableLockType lock;
        if (_waitingJobs.empty())
        {
            // free the incoming job entry so that it's available for another job
            _inJobs[slotIdx] = nullptr;
        }
        else
        {
            // trigger next job from the waiting list, reusing the already reserved slotIdx
            StorageJob& nextJob = _waitingJobs.front();
            _waitingJobs.pop_front();
            _inJobs[slotIdx] = &nextJob;
            lock.unlock();
            triggerJob(nextJob, *(getConfigEntry(nextJob.getId())), slotIdx);
        }
    }
}

MappingConfig const* MappingStorage::getConfigEntry(uint32_t const blockId) const
{
    // binary search (NOTE: entries in _config must be sorted by ascending blockId)
    auto const cmp
        = [](MappingConfig const& entry, uint32_t const id) { return entry.blockId < id; };
    auto const* const configEnd = _config + _configSize;
    auto const* const it        = etl::lower_bound(_config, configEnd, blockId, cmp);
    if ((it != configEnd) && (it->blockId == blockId))
    {
        return it;
    }
    return nullptr;
}

size_t MappingStorage::getUsedSlotIdx(StorageJob& job) const
{
    // go through the slots and find the used job; we could also try to subtract _outJobs start
    // address from the job address, but that would make it harder to detect unknown jobs
    auto const maxSlot = _outJobs.size();
    for (size_t slotIdx = 0U; slotIdx < maxSlot; ++slotIdx)
    {
        if (_outJobs[slotIdx] == (&job))
        {
            return slotIdx;
        }
    }
    return maxSlot;
}

} // namespace storage
