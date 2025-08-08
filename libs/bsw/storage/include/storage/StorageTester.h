// Copyright 2025 Accenture.

#pragma once

#include "storage/IStorage.h"
#include "storage/StorageJob.h"

#include <async/FutureSupport.h>
#include <etl/array.h>
#include <etl/span.h>
#include <util/command/GroupCommand.h>
#include <util/format/SharedStringWriter.h>

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
        IStorage& storage, ::async::ContextType context, ::etl::span<uint8_t> dataBuf);

    DECLARE_COMMAND_GROUP_GET_INFO
    void executeCommand(::util::command::CommandContext& context, uint8_t idx) override;

private:
    void printData(::util::format::SharedStringWriter& out, ::etl::span<uint8_t> data);
    void jobDone(StorageJob&);

    IStorage& _storage;
    ::storage::StorageJob::JobDoneCallback const _jobDone;
    ::async::FutureSupport _future;
    ::etl::span<uint8_t> const _dataBuf;
};

namespace declare
{
// MAX_DATA_SIZE: maximum block data size
template<size_t MAX_DATA_SIZE>
class StorageTester : public ::storage::StorageTester
{
    static_assert(MAX_DATA_SIZE > 0U, "maximum block data size must be bigger than 0");

public:
    explicit StorageTester(IStorage& storage, ::async::ContextType const context)
    : ::storage::StorageTester(storage, context, _dataBuf)
    {}

private:
    ::etl::array<uint8_t, MAX_DATA_SIZE> _dataBuf;
};
} // namespace declare

} // namespace storage
