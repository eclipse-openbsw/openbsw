// Copyright 2025 Accenture.

#include <storage/StorageTester.h>
#include <util/format/SharedStringWriter.h>

#include <cstring>

namespace storage
{

static constexpr uint8_t WRITE_CMD = 1U;
static constexpr uint8_t READ_CMD  = 2U;

DEFINE_COMMAND_GROUP_GET_INFO_BEGIN(StorageTester, "storage", "Storage testing")
COMMAND_GROUP_COMMAND(
    WRITE_CMD, "write", "write <id in dec> <value in hex>, example: storage write 2561 aabb")
COMMAND_GROUP_COMMAND(
    READ_CMD, "read", "read <id in dec> <size in bytes, default: 1>, example: storage read 2561 2")
DEFINE_COMMAND_GROUP_GET_INFO_END

StorageTester::StorageTester(
    IStorage& storage,
    ::async::ContextType const context,
    uint8_t* const dataBuf,
    size_t const dataBufSize)
: _storage(storage)
, _jobDone(StorageJob::JobDoneCallback::create<StorageTester, &StorageTester::jobDone>(*this))
, _future(context)
, _dataBuf(dataBuf)
, _dataBufSize(dataBufSize)
{}

void StorageTester::executeCommand(::util::command::CommandContext& context, uint8_t const idx)
{
    ::util::format::SharedStringWriter out(context);
    switch (idx)
    {
        case WRITE_CMD:
        {
            char const* const usageStr = "usage: storage write <id> <value in hex>\n";
            if (!context.check(context.hasToken(), ICommand::Result::BAD_TOKEN))
            {
                (void)out.printf(usageStr);
                return;
            }
            auto const id = context.scanIntToken<uint32_t>();

            if (!context.check(context.hasToken(), ICommand::Result::BAD_TOKEN))
            {
                (void)out.printf(usageStr);
                return;
            }
            auto data = context.scanByteBufferToken(
                ::estd::slice<uint8_t>::from_pointer(_dataBuf, _dataBufSize));

            _storageWriteBuf.setBuffer(::estd::slice<uint8_t const>::from_pointer(
                reinterpret_cast<uint8_t const*>(_dataBuf), data.size()));
            _storageJob.init(id, _jobDone);
            _storageJob.initWrite(_storageWriteBuf, 0U);
            _storage.process(_storageJob);
            _future.wait();
            if (_storageJob.getResult().is<::storage::StorageJob::Result::Success>())
            {
                (void)out.printf("Writing ok\r\n");
            }
            else
            {
                (void)out.printf("Writing failed: %d\r\n", _storageJob.getResult().index());
            }
            break;
        }
        case READ_CMD:
        {
            if (!context.check(context.hasToken(), ICommand::Result::BAD_TOKEN))
            {
                (void)out.printf("usage: storage read <id> <size in bytes>\r\n");
                return;
            }
            auto const id = context.scanIntToken<uint32_t>();

            size_t dataSize = 1U;
            if (context.hasToken())
            {
                dataSize = context.scanIntToken<size_t>();
            }
            if ((dataSize == 0U) || (dataSize > sizeof(_dataBuf)))
            {
                (void)out.printf("ERROR: invalid read size");
                return;
            }

            (void)memset(_dataBuf, 0U, dataSize);
            _storageReadBuf.setBuffer(::estd::slice<uint8_t>::from_pointer(_dataBuf, dataSize));
            _storageJob.init(id, _jobDone);
            _storageJob.initRead(_storageReadBuf, 0U);
            _storage.process(_storageJob);
            _future.wait();
            if (!_storageJob.getResult().is<::storage::StorageJob::Result::Success>())
            {
                (void)out.printf("Reading failed: %d\r\n", _storageJob.getResult().index());
                return;
            }
            auto const readSize = _storageJob.getRead().getReadSize();
            (void)out.printf("Reading ok, size: %d, data:\r\n\r\n", readSize);
            for (size_t i = 0U; i < readSize; ++i)
            {
                (void)out.printf(" %d: 0x%x\r\n", i, _dataBuf[i]);
            }
            (void)out.printf("\r\n");
            break;
        }
        default:
        {
            break;
        }
    }
}

void StorageTester::jobDone(StorageJob&) { _future.notify(); }

} /* namespace storage */
