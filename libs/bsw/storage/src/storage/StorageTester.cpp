// Copyright 2025 Accenture.

#include <etl/memory.h>
#include <storage/StorageTester.h>

namespace storage
{

static constexpr uint8_t WRITE_CMD = 1U;
static constexpr uint8_t FILL_CMD  = 2U;
static constexpr uint8_t READ_CMD  = 3U;

DEFINE_COMMAND_GROUP_GET_INFO_BEGIN(StorageTester, "storage", "Storage testing")
COMMAND_GROUP_COMMAND(
    WRITE_CMD, "write", "write <id> <values in hex>, example: storage write 2561 aabb")
COMMAND_GROUP_COMMAND(
    FILL_CMD, "fill", "fill <id> <value> <size in bytes>, example: storage fill 2561 0xab 2")
COMMAND_GROUP_COMMAND(
    READ_CMD, "read", "read <id> <size in bytes, default: all>, example: storage read 2561 2")
DEFINE_COMMAND_GROUP_GET_INFO_END

StorageTester::StorageTester(
    IStorage& storage, ::async::ContextType const context, ::etl::span<uint8_t> const dataBuf)
: _storage(storage)
, _jobDone(StorageJob::JobDoneCallback::create<StorageTester, &StorageTester::jobDone>(*this))
, _future(context)
, _dataBuf(dataBuf)
{}

void StorageTester::executeCommand(::util::command::CommandContext& context, uint8_t const idx)
{
    ::util::format::SharedStringWriter out(context);
    StorageJob job;
    switch (idx)
    {
        case WRITE_CMD:
        {
            char const* const usageStr = "usage: storage write <id> <values in hex>\r\n";
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
            auto const values = context.scanByteBufferToken(_dataBuf);

            StorageJob::Type::Write::BufferType writeBuffer(_dataBuf.first(values.size()));
            job.init(id, _jobDone);
            job.initWrite(writeBuffer, 0U);
            _storage.process(job);
            _future.wait();
            if (job.hasResult<::storage::StorageJob::Result::Success>())
            {
                (void)out.printf("Writing ok\r\n");
            }
            else
            {
                (void)out.printf("Writing failed: %d\r\n", job.getResult().index());
            }
            break;
        }
        case FILL_CMD:
        {
            char const* const usageStr = "usage: storage fill <id> <value> <size in bytes>\r\n";
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

            if (!context.check(context.hasToken(), ICommand::Result::BAD_TOKEN))
            {
                (void)out.printf(usageStr);
                return;
            }
            auto const value = context.scanIntToken<uint8_t>();

            size_t dataSize = 0U;
            if (context.hasToken())
            {
                dataSize = context.scanIntToken<size_t>();
            }
            if ((dataSize == 0U) || (dataSize > _dataBuf.size()))
            {
                (void)out.printf("ERROR: invalid fill size\r\n");
                return;
            }

            auto const data = _dataBuf.first(dataSize);
            (void)::etl::mem_set(data.data(), data.size(), value);
            StorageJob::Type::Write::BufferType writeBuffer(data);
            job.init(id, _jobDone);
            job.initWrite(writeBuffer, 0U);
            _storage.process(job);
            _future.wait();
            if (job.hasResult<::storage::StorageJob::Result::Success>())
            {
                (void)out.printf("Fill ok\r\n");
            }
            else
            {
                (void)out.printf("Fill failed: %d\r\n", job.getResult().index());
            }
            break;
        }
        case READ_CMD:
        {
            if (!context.check(context.hasToken(), ICommand::Result::BAD_TOKEN))
            {
                (void)out.printf("usage: storage read <id> <size in bytes, default: all>\r\n");
                return;
            }
            auto const id = context.scanIntToken<uint32_t>();

            auto dataSize = _dataBuf.size();
            if (context.hasToken())
            {
                dataSize = context.scanIntToken<size_t>();
            }
            if ((dataSize == 0U) || (dataSize > _dataBuf.size()))
            {
                (void)out.printf("ERROR: invalid read size\r\n");
                return;
            }

            auto const data = _dataBuf.first(dataSize);
            (void)::etl::mem_set(data.data(), data.size(), static_cast<uint8_t>(0U));
            StorageJob::Type::Read::BufferType readBuffer(data);
            job.init(id, _jobDone);
            job.initRead(readBuffer, 0U);
            _storage.process(job);
            _future.wait();
            if (!job.hasResult<::storage::StorageJob::Result::Success>())
            {
                (void)out.printf("Reading failed: %d\r\n", job.getResult().index());
                return;
            }
            auto const readSize = job.getRead().getReadSize();
            (void)out.printf("Reading ok, size: %d, data:\r\n\r\n", readSize);
            printData(out, data);
            break;
        }
        default:
        {
            break;
        }
    }
}

void StorageTester::printData(
    ::util::format::SharedStringWriter& out, ::etl::span<uint8_t> const data)
{
    static constexpr uint32_t BYTES_PER_LINE = 32U;
    for (uint32_t i = 0U; i < data.size(); i += BYTES_PER_LINE)
    {
        (void)out.printf("%-5d: ", i);
        for (uint32_t j = 0U; j < BYTES_PER_LINE; ++j)
        {
            if (i + j < data.size())
            {
                (void)out.printf("%02x", data[i + j]);
            }
            if (((j + 1) & 3U) == 0U)
            {
                (void)out.printf(" ");
            }
        }
        (void)out.printf("\r\n");
    }
    (void)out.printf("\r\n");
}

void StorageTester::jobDone(StorageJob&) { _future.notify(); }

} // namespace storage
