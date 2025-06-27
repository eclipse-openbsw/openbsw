// Copyright 2025 Accenture.

#pragma once

#include "storage/IStorage.h"
#include "storage/StorageJob.h"

#include <async/FutureSupport.h>
#include <util/command/GroupCommand.h>

namespace storage
{

class StorageTester : public ::util::command::GroupCommand
{
public:
    ~StorageTester()                               = default;
    StorageTester(StorageTester const&)            = delete;
    StorageTester& operator=(StorageTester const&) = delete;

protected:
    explicit StorageTester(
        IStorage& storage, ::async::ContextType context, uint8_t* dataBuf, size_t dataBufSize);

    DECLARE_COMMAND_GROUP_GET_INFO
    void executeCommand(::util::command::CommandContext& context, uint8_t idx) override;

private:
    void jobDone(StorageJob&);

    IStorage& _storage;
    StorageJob _storageJob;
    ::util::LinkedBuffer<uint8_t> _storageReadBuf;
    ::util::LinkedBuffer<uint8_t const> _storageWriteBuf;
    ::storage::StorageJob::JobDoneCallback const _jobDone;
    ::async::FutureSupport _future;
    uint8_t* const _dataBuf;
    size_t const _dataBufSize;
};

namespace declare
{
// MAX_BLOCK_SIZE: maximum block data size
template<size_t MAX_BLOCK_SIZE>
class StorageTester : public ::storage::StorageTester
{
    static_assert(MAX_BLOCK_SIZE > 0U, "maximum block size must be higher than 0");

public:
    explicit StorageTester(IStorage& storage, ::async::ContextType const context)
    : ::storage::StorageTester(storage, context, _dataBuf, MAX_BLOCK_SIZE)
    {}

private:
    uint8_t _dataBuf[MAX_BLOCK_SIZE];
};
} // namespace declare

} /* namespace storage */
