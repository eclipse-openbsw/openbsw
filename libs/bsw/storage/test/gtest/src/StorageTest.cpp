// Copyright 2025 Accenture.

#include <async/Async.h>
#include <async/AsyncMock.h>
#include <async/TestContext.h>
#include <bsp/eeprom/EepromDriverMock.h>
#include <storage/EepStorage.h>
#include <storage/FeeStorage.h>
#include <storage/IStorageMock.h>
#include <storage/LinkedBuffer.h>
#include <storage/MappingStorage.h>
#include <storage/QueuingStorage.h>
#include <storage/StorageJob.h>
#include <util/estd/assert.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace
{
using namespace ::testing;

// NOTE: storing test results in an array requires the IDs to be continuous and in ascending order
static uint32_t const BLOCKID1      = 100U;
static uint32_t const BLOCKID2      = 101U;
static uint32_t const BLOCKID3      = 102U;
static uint32_t const BLOCKID4      = 103U;
static uint32_t const BLOCKID5      = 104U;
static uint32_t const BLOCKID6      = 105U;
static uint32_t const BLOCKID7      = 106U;
static uint32_t const BLOCKID8      = 107U;
static uint8_t const EEPSTORAGEIDX  = 0U;
static uint8_t const FEESTORAGEIDX  = 1U;
static uint8_t const MOCKSTORAGEIDX = 2U;

static constexpr ::storage::MappingConfig MAPPING_CONFIG[] = {
    {BLOCKID1, 0U, EEPSTORAGEIDX},
    {BLOCKID2, 0U, FEESTORAGEIDX},
    {BLOCKID3, 1U, EEPSTORAGEIDX},
    {BLOCKID4, 2U, EEPSTORAGEIDX},
    {BLOCKID5, 3U, EEPSTORAGEIDX},
    {BLOCKID6, 0U, MOCKSTORAGEIDX},
    {BLOCKID7, 4U, EEPSTORAGEIDX},
    {BLOCKID8, 0U, 3U}, // storage index exceeds the limit given for MappingStorage
};

static constexpr ::storage::EepBlockConfig EEP_BLOCK_CONFIG[] = {
    {0U /* address */, 2U /* size (without 4-byte header) */, true /* error detection */},
    {10U, 2U, true},
    {20U, 4U, false},
    {30U, 4U, true},
    {38U, 5U, true}, // invalid block size (bigger than the limit given for eepStorage)
};

class StorageTest : public Test
{
public:
    using StorageJob = ::storage::StorageJob;

    StorageTest()
    : eepStorage(EEP_BLOCK_CONFIG, eepMock)
    , eepQueuingStorage(eepStorage, context)
    , feeQueuingStorage(feeStorage, context)
    , storage(
          MAPPING_CONFIG,
          context,
          eepQueuingStorage /* storage index 0 */,
          feeQueuingStorage /* 1 */,
          storageMock /* 2 */)
    , jobDoneCb(StorageJob::JobDoneCallback::create<StorageTest, &StorageTest::jobDoneFunc>(*this))
    {
        (void)memset(jobResults, 0U, sizeof(jobResults));
        (void)memset(eepData, EEP_TESTVAL, sizeof(eepData));
        // store correct, precalculated checksums for reading that match the default data
        eepData[0U]  = 0xD3U; // BLOCKID1
        eepData[1U]  = 0xDCU;
        eepData[10U] = 0xD3U; // BLOCKID3
        eepData[11U] = 0xDCU;
        eepData[30U] = 0x36U; // BLOCKID5
        eepData[31U] = 0x18U;
        // used sizes
        eepData[2U]  = 0U;
        eepData[3U]  = 2U;
        eepData[12U] = 0U;
        eepData[13U] = 2U;
        eepData[32U] = 0U;
        eepData[33U] = 1U; // NOTE: smaller than the defined blockSize
    }

    void eepRead(uint32_t address, uint8_t* dst, uint32_t length)
    {
        estd_assert((address + length) <= sizeof(eepData));
        (void)memcpy(dst, &(eepData[address]), length);
    }

    void eepWrite(uint32_t address, uint8_t const* src, uint32_t length)
    {
        estd_assert((address + length) <= sizeof(eepData));
        (void)memcpy(&(eepData[address]), src, length);
    }

    void processCallsCbTwice(StorageJob& job)
    {
        job.sendResult(StorageJob::Result::Success());
        job.sendResult(StorageJob::Result::Success());
    }

    void jobDoneFunc(StorageJob& job)
    {
        ++(jobResults[(job.getId() - BLOCKID1)][job.getResult().index()]);
    }

    template<typename T>
    bool hasResult(uint32_t const id, uint8_t const times)
    {
        T const result;
        StorageJob::ResultType const resultVar(result);
        if (jobResults[(id - BLOCKID1)][resultVar.index()] == times)
        {
            return true;
        }
        return false;
    }

    bool hasSucceeded(uint32_t const id, uint8_t const times = 1U)
    {
        return hasResult<StorageJob::Result::Success>(id, times);
    }

    bool hasFailed(uint32_t const id, uint8_t const times = 1U)
    {
        return hasResult<StorageJob::Result::Error>(id, times);
    }

protected:
    StrictMock<::async::AsyncMock> asyncMock;
    ::async::TestContext context{1};
    ::eeprom::EepromDriverMock eepMock;
    ::storage::declare::EepStorage<
        (sizeof(EEP_BLOCK_CONFIG) / sizeof(::storage::EepBlockConfig)),
        4U /* max block size */>
        eepStorage;
    ::storage::FeeStorage feeStorage;
    ::storage::IStorageMock storageMock;
    ::storage::QueuingStorage eepQueuingStorage;
    ::storage::QueuingStorage feeQueuingStorage;
    ::storage::declare::MappingStorage<
        (sizeof(MAPPING_CONFIG) / sizeof(::storage::MappingConfig)),
        3U /* number of outgoing storages */,
        2U /* max outgoing jobs */>
        storage;
    static constexpr size_t EEP_LAYOUT_SIZE = 38U;
    uint8_t eepData[EEP_LAYOUT_SIZE];

    static_assert(BLOCKID8 > BLOCKID1, "block IDs must be defined in ascending order");
    uint8_t jobResults[(BLOCKID8 - BLOCKID1 + 1)][4U /* number of result kinds */];
    StorageJob::JobDoneCallback const jobDoneCb;

    uint8_t const EEP_TESTVAL = 111U;
    uint8_t const FEE_TESTVAL = 222U;
    uint8_t const WRITEVAL    = 90U;
    uint8_t const WRITEVAL2   = 100U;
};

TEST_F(StorageTest, MultipleJobsAtOnce)
{
    context.handleExecute();

    uint8_t data[] = {0U, 0U};
    ::etl::span<uint8_t> dataSpan(data);
    StorageJob::Type::Read::BufferType buf(dataSpan);

    StorageJob job;
    job.init(BLOCKID1, jobDoneCb);
    job.initRead(buf);
    storage.process(job);

    uint8_t data2[] = {0U};
    ::etl::span<uint8_t> dataSpan2(data2);
    StorageJob::Type::Read::BufferType buf2(dataSpan2);

    StorageJob job2;
    job2.init(BLOCKID2, jobDoneCb);
    job2.initRead(buf2);
    storage.process(job2);

    uint8_t const data3[] = {0U};
    ::etl::span<uint8_t const> dataSpan3(data3);
    StorageJob::Type::Write::BufferType buf3(dataSpan3);

    StorageJob job3;
    job3.init(BLOCKID2, jobDoneCb);
    job3.initWrite(buf3);
    // NOTE: storage has only has 2 job slots so it will put this job into the waiting list
    storage.process(job3);

    EXPECT_CALL(eepMock, read(0U, _, 6U))
        .WillOnce(DoAll(Invoke(this, &StorageTest::eepRead), Return(::bsp::BSP_OK)));
    // NOTE: FeeStorage still not implemented, no FEE mock needed
    context.execute();

    EXPECT_TRUE(hasSucceeded(BLOCKID1));
    EXPECT_TRUE(hasSucceeded(BLOCKID2, 2U));
    EXPECT_EQ(job.getRead().getReadSize(), dataSpan.size());
    EXPECT_EQ(job2.getRead().getReadSize(), dataSpan2.size());
    EXPECT_EQ(data[0U], EEP_TESTVAL);
    EXPECT_EQ(data[1U], EEP_TESTVAL);
    EXPECT_EQ(data2[0U], FEE_TESTVAL);
}

TEST_F(StorageTest, ReadWithChecksumIntoSmallerBuffer)
{
    context.handleExecute();

    uint8_t data[] = {0U};
    ::etl::span<uint8_t> dataSpan(data);
    StorageJob::Type::Read::BufferType buf(dataSpan);

    StorageJob job;
    job.init(BLOCKID3, jobDoneCb);
    job.initRead(buf);
    storage.process(job);

    EXPECT_CALL(eepMock, read(10U, _, 6U))
        .WillOnce(DoAll(Invoke(this, &StorageTest::eepRead), Return(::bsp::BSP_OK)));
    context.execute();

    EXPECT_TRUE(hasSucceeded(BLOCKID3));
    EXPECT_EQ(job.getRead().getReadSize(), dataSpan.size());
    EXPECT_EQ(data[0U], EEP_TESTVAL);
}

TEST_F(StorageTest, ReadWithChecksumIntoBiggerBuffer)
{
    context.handleExecute();

    uint8_t data[] = {0U, 0U};
    ::etl::span<uint8_t> dataSpan(data);
    StorageJob::Type::Read::BufferType buf(dataSpan);

    StorageJob job;
    job.init(BLOCKID5, jobDoneCb);
    job.initRead(buf);
    storage.process(job);

    EXPECT_CALL(eepMock, read(30U, _, 8U))
        .WillOnce(DoAll(Invoke(this, &StorageTest::eepRead), Return(::bsp::BSP_OK)));
    context.execute();

    EXPECT_TRUE(hasSucceeded(BLOCKID5));
    EXPECT_EQ(job.getRead().getReadSize(), 1U);
    EXPECT_EQ(data[0U], EEP_TESTVAL);
}

TEST_F(StorageTest, WriteWithChecksumFromSmallerBuffer)
{
    context.handleExecute();

    uint8_t const data[] = {WRITEVAL};
    ::etl::span<uint8_t const> dataSpan(data);
    StorageJob::Type::Write::BufferType buf(dataSpan);

    StorageJob job;
    job.init(BLOCKID3, jobDoneCb);
    job.initWrite(buf);
    storage.process(job);

    {
        InSequence const seq;
        EXPECT_CALL(eepMock, read(10U, _, 6U))
            .WillOnce(DoAll(Invoke(this, &StorageTest::eepRead), Return(::bsp::BSP_OK)));
        EXPECT_CALL(eepMock, write(10U, _, 5U))
            .WillOnce(DoAll(Invoke(this, &StorageTest::eepWrite), Return(::bsp::BSP_OK)));
    }
    context.execute();

    EXPECT_TRUE(hasSucceeded(BLOCKID3));
    EXPECT_EQ(eepData[10U], 0x29U); // verify checksum
    EXPECT_EQ(eepData[11U], 0xBCU);
    EXPECT_EQ(eepData[12U], 0U);
    EXPECT_EQ(eepData[13U], 2U);       // size should still be 2
    EXPECT_EQ(eepData[14U], WRITEVAL); // only the 1st data byte has been written
    EXPECT_EQ(eepData[15U], EEP_TESTVAL);
}

TEST_F(StorageTest, ReadIntoTwoBuffers)
{
    context.handleExecute();

    uint8_t data[] = {0U, 0U};
    ::etl::span<uint8_t> dataSpan(data);
    StorageJob::Type::Read::BufferType buf(dataSpan);
    uint8_t data2[] = {0U, 0U};
    ::etl::span<uint8_t> dataSpan2(data2);
    StorageJob::Type::Read::BufferType buf2(dataSpan2);
    buf.setNext(&buf2);

    StorageJob job;
    job.init(BLOCKID4, jobDoneCb);
    job.initRead(buf);
    storage.process(job);

    EXPECT_CALL(eepMock, read(20U, _, 4U))
        .WillOnce(DoAll(Invoke(this, &StorageTest::eepRead), Return(::bsp::BSP_OK)));
    context.execute();

    EXPECT_TRUE(hasSucceeded(BLOCKID4));
    EXPECT_EQ(job.getRead().getReadSize(), (dataSpan.size() + dataSpan2.size()));
    EXPECT_EQ(data[0U], EEP_TESTVAL);
    EXPECT_EQ(data[1U], EEP_TESTVAL);
    EXPECT_EQ(data2[0U], EEP_TESTVAL);
    EXPECT_EQ(data2[1U], EEP_TESTVAL);
}

TEST_F(StorageTest, WriteFromTwoBuffers)
{
    context.handleExecute();

    uint8_t const data[] = {WRITEVAL, WRITEVAL};
    ::etl::span<uint8_t const> dataSpan(data);
    StorageJob::Type::Write::BufferType buf(dataSpan);
    uint8_t const data2[] = {WRITEVAL2, WRITEVAL2};
    ::etl::span<uint8_t const> dataSpan2(data2);
    StorageJob::Type::Write::BufferType buf2(dataSpan2);
    buf.setNext(&buf2);

    StorageJob job;
    job.init(BLOCKID4, jobDoneCb);
    job.initWrite(buf);
    storage.process(job);

    EXPECT_CALL(eepMock, write(20U, _, 4U))
        .WillOnce(DoAll(Invoke(this, &StorageTest::eepWrite), Return(::bsp::BSP_OK)));
    context.execute();

    EXPECT_TRUE(hasSucceeded(BLOCKID4));
    EXPECT_EQ(eepData[20U], WRITEVAL);
    EXPECT_EQ(eepData[21U], WRITEVAL);
    EXPECT_EQ(eepData[22U], WRITEVAL2);
    EXPECT_EQ(eepData[23U], WRITEVAL2);
}

TEST_F(StorageTest, ReadFromOffset)
{
    context.handleExecute();

    uint8_t data[] = {0U};
    ::etl::span<uint8_t> dataSpan(data);
    StorageJob::Type::Read::BufferType buf(dataSpan);

    StorageJob job;
    job.init(BLOCKID3, jobDoneCb);
    job.initRead(buf, 1U);
    storage.process(job);

    EXPECT_CALL(eepMock, read(10U, _, 6U))
        .WillOnce(DoAll(Invoke(this, &StorageTest::eepRead), Return(::bsp::BSP_OK)));
    ++eepData[15U];       // change the 2nd data byte so that we can distinguish it from the 1st
    eepData[10U] = 0x30U; // update checksum
    eepData[11U] = 0x2U;
    context.execute();

    EXPECT_TRUE(hasSucceeded(BLOCKID3));
    EXPECT_EQ(job.getRead().getReadSize(), dataSpan.size());
    EXPECT_EQ(data[0U], (EEP_TESTVAL + 1U));
}

TEST_F(StorageTest, WriteToOffset)
{
    context.handleExecute();

    uint8_t const data[] = {WRITEVAL};
    ::etl::span<uint8_t const> dataSpan(data);
    StorageJob::Type::Write::BufferType buf(dataSpan);

    StorageJob job;
    job.init(BLOCKID3, jobDoneCb);
    job.initWrite(buf, 1U);
    storage.process(job);

    {
        InSequence const seq;
        EXPECT_CALL(eepMock, read(10U, _, 6U))
            .WillOnce(DoAll(Invoke(this, &StorageTest::eepRead), Return(::bsp::BSP_OK)));
        EXPECT_CALL(eepMock, write(10U, _, 6U))
            .WillOnce(DoAll(Invoke(this, &StorageTest::eepWrite), Return(::bsp::BSP_OK)));
    }
    context.execute();

    EXPECT_TRUE(hasSucceeded(BLOCKID3));
    EXPECT_EQ(eepData[10U], 0xB5U); // verify checksum
    EXPECT_EQ(eepData[11U], 0x2AU);
    EXPECT_EQ(eepData[12U], 0U); // verify size
    EXPECT_EQ(eepData[13U], 2U);
    EXPECT_EQ(eepData[14U], EEP_TESTVAL);
    EXPECT_EQ(eepData[15U], WRITEVAL);
}

TEST_F(StorageTest, WriteToOffsetWithoutExistingData)
{
    context.handleExecute();

    uint8_t const data[] = {WRITEVAL};
    ::etl::span<uint8_t const> dataSpan(data);
    StorageJob::Type::Write::BufferType buf(dataSpan);

    StorageJob job;
    job.init(BLOCKID3, jobDoneCb);
    job.initWrite(buf, 1U);
    storage.process(job);

    {
        InSequence const seq;
        EXPECT_CALL(eepMock, read(10U, _, 6U))
            .WillOnce(DoAll(Invoke(this, &StorageTest::eepRead), Return(::bsp::BSP_OK)));
        EXPECT_CALL(eepMock, write(10U, _, 6U))
            .WillOnce(DoAll(Invoke(this, &StorageTest::eepWrite), Return(::bsp::BSP_OK)));
    }
    ++eepData[10U]; // invalidate existing checksum
    context.execute();

    EXPECT_TRUE(hasSucceeded(BLOCKID3));
    EXPECT_EQ(eepData[10U], 0xAEU); // verify checksum
    EXPECT_EQ(eepData[11U], 0x3EU);
    EXPECT_EQ(eepData[12U], 0U); // verify size
    EXPECT_EQ(eepData[13U], 2U);
    EXPECT_EQ(eepData[14U], 0U); // 1st data byte initialized to default
    EXPECT_EQ(eepData[15U], WRITEVAL);
}

TEST_F(StorageTest, ReadError)
{
    context.handleExecute();

    uint8_t data[] = {0U};
    ::etl::span<uint8_t> dataSpan(data);
    StorageJob::Type::Read::BufferType buf(dataSpan);

    StorageJob job;
    job.init(BLOCKID3, jobDoneCb);
    job.initRead(buf);
    storage.process(job);

    EXPECT_CALL(eepMock, read(10U, _, 6U))
        .WillOnce(DoAll(Invoke(this, &StorageTest::eepRead), Return(::bsp::BSP_ERROR)));
    context.execute();

    EXPECT_TRUE(hasFailed(BLOCKID3));
    EXPECT_EQ(job.getRead().getReadSize(), 0U);
}

TEST_F(StorageTest, WriteError)
{
    context.handleExecute();

    uint8_t const data[] = {0U};
    ::etl::span<uint8_t const> dataSpan(data);
    StorageJob::Type::Write::BufferType buf(dataSpan);

    StorageJob job;
    job.init(BLOCKID3, jobDoneCb);
    job.initWrite(buf);
    storage.process(job);

    EXPECT_CALL(eepMock, read(10U, _, 6U))
        .WillOnce(DoAll(Invoke(this, &StorageTest::eepRead), Return(::bsp::BSP_ERROR)));
    context.execute();
    EXPECT_TRUE(hasFailed(BLOCKID3));

    storage.process(job);
    {
        InSequence const seq;
        EXPECT_CALL(eepMock, read(10U, _, 6U))
            .WillOnce(DoAll(Invoke(this, &StorageTest::eepRead), Return(::bsp::BSP_OK)));
        EXPECT_CALL(eepMock, write(10U, _, 5U))
            .WillOnce(DoAll(Invoke(this, &StorageTest::eepWrite), Return(::bsp::BSP_ERROR)));
    }
    context.execute();
    EXPECT_TRUE(hasFailed(BLOCKID3, 2U));
}

TEST_F(StorageTest, InvalidChecksum)
{
    context.handleExecute();

    uint8_t data[] = {0U, 0U};
    ::etl::span<uint8_t> dataSpan(data);
    StorageJob::Type::Read::BufferType buf(dataSpan);

    StorageJob job;
    job.init(BLOCKID1, jobDoneCb);
    job.initRead(buf);
    storage.process(job);

    EXPECT_CALL(eepMock, read(0U, _, 6U))
        .WillOnce(DoAll(Invoke(this, &StorageTest::eepRead), Return(::bsp::BSP_OK)));
    ++eepData[0U]; // invalidate the checksum
    context.execute();
    EXPECT_TRUE(hasResult<StorageJob::Result::DataLoss>(BLOCKID1, 1U));
}

TEST_F(StorageTest, InvalidReadOffset)
{
    context.handleExecute();

    uint8_t data[] = {0U};
    ::etl::span<uint8_t> dataSpan(data);
    StorageJob::Type::Read::BufferType buf(dataSpan);

    StorageJob job;
    job.init(BLOCKID3, jobDoneCb);
    job.initRead(buf, 2U);
    storage.process(job);

    EXPECT_CALL(eepMock, read(10U, _, 6U))
        .WillOnce(DoAll(Invoke(this, &StorageTest::eepRead), Return(::bsp::BSP_OK)));
    context.execute();
    EXPECT_TRUE(hasSucceeded(BLOCKID3));
    EXPECT_EQ(job.getRead().getReadSize(), 0U);

    job.initRead(buf, 3U);
    storage.process(job);

    EXPECT_CALL(eepMock, read(10U, _, 6U))
        .WillOnce(DoAll(Invoke(this, &StorageTest::eepRead), Return(::bsp::BSP_OK)));
    context.execute();
    EXPECT_TRUE(hasSucceeded(BLOCKID3, 2U));
    EXPECT_EQ(job.getRead().getReadSize(), 0U);
}

TEST_F(StorageTest, InvalidWriteSize)
{
    context.handleExecute();

    uint8_t const data[] = {WRITEVAL, WRITEVAL};
    ::etl::span<uint8_t const> dataSpan(data);
    StorageJob::Type::Write::BufferType buf(dataSpan);

    StorageJob job;
    job.init(BLOCKID3, jobDoneCb);
    job.initWrite(buf, 1U);
    storage.process(job);

    EXPECT_CALL(eepMock, read(10U, _, 6U))
        .WillOnce(DoAll(Invoke(this, &StorageTest::eepRead), Return(::bsp::BSP_OK)));
    context.execute();
    EXPECT_TRUE(hasFailed(BLOCKID3));
}

TEST_F(StorageTest, EmptyWrite)
{
    context.handleExecute();

    ::etl::span<uint8_t const> dataSpan;
    StorageJob::Type::Write::BufferType buf(dataSpan);

    StorageJob job;
    job.init(BLOCKID3, jobDoneCb);
    job.initWrite(buf, 1U);
    storage.process(job);

    EXPECT_CALL(eepMock, read(10U, _, 6U))
        .WillOnce(DoAll(Invoke(this, &StorageTest::eepRead), Return(::bsp::BSP_OK)));
    context.execute();
    EXPECT_TRUE(hasFailed(BLOCKID3));
}

TEST_F(StorageTest, InvalidWriteOffset)
{
    context.handleExecute();

    uint8_t const data[] = {WRITEVAL};
    ::etl::span<uint8_t const> dataSpan(data);
    StorageJob::Type::Write::BufferType buf(dataSpan);

    StorageJob job;
    job.init(BLOCKID3, jobDoneCb);
    job.initWrite(buf, 2U);
    storage.process(job);
    context.execute();
    EXPECT_TRUE(hasFailed(BLOCKID3));

    job.initWrite(buf, 3U);
    storage.process(job);
    context.execute();
    EXPECT_TRUE(hasFailed(BLOCKID3, 2U));
}

TEST_F(StorageTest, InvalidBlockId)
{
    context.handleExecute();

    StorageJob::ResultType result = StorageJob::Result::Init();
    auto cbk                      = [&result](StorageJob& job)
    {
        estd_assert(job.getId() == 5U);
        result = job.getResult();
    };

    uint8_t data[] = {0U};
    ::etl::span<uint8_t> dataSpan(data);
    StorageJob::Type::Read::BufferType buf(dataSpan);

    StorageJob job;
    job.init(5U, StorageJob::JobDoneCallback(cbk));
    job.initRead(buf);
    storage.process(job);
    context.execute();
    EXPECT_TRUE(::etl::holds_alternative<StorageJob::Result::Error>(result));
    EXPECT_TRUE(job.hasResult<StorageJob::Result::Error>());

    // try calling eepStorage directly
    result = StorageJob::Result::Init();
    eepStorage.process(job);
    EXPECT_TRUE(::etl::holds_alternative<StorageJob::Result::Error>(result));
    EXPECT_TRUE(job.hasResult<StorageJob::Result::Error>());
}

TEST_F(StorageTest, InvalidBlockSize)
{
    context.handleExecute();

    uint8_t data[] = {0U};
    ::etl::span<uint8_t> dataSpan(data);
    StorageJob::Type::Read::BufferType buf(dataSpan);

    StorageJob job;
    job.init(BLOCKID7, jobDoneCb);
    job.initRead(buf);
    storage.process(job);
    context.execute();
    EXPECT_TRUE(hasFailed(BLOCKID7));
}

TEST_F(StorageTest, InvalidStorageIdx)
{
    context.handleExecute();

    uint8_t data[] = {0U};
    ::etl::span<uint8_t> dataSpan(data);
    StorageJob::Type::Read::BufferType buf(dataSpan);

    StorageJob job;
    job.init(BLOCKID8, jobDoneCb);
    job.initRead(buf);
    storage.process(job);
    context.execute();
    EXPECT_TRUE(hasFailed(BLOCKID8));
}

TEST_F(StorageTest, MultipleErrors)
{
    context.handleExecute();

    StorageJob job;
    StorageJob job2;
    job.init(BLOCKID1, jobDoneCb);
    job2.init(BLOCKID2, jobDoneCb);
    storage.process(job);
    storage.process(job2);
    // NOTE: at this point there the mapper will have two items in the failed jobs list
    context.execute();
    EXPECT_TRUE(hasFailed(BLOCKID1));
    EXPECT_TRUE(hasFailed(BLOCKID2));
}

TEST_F(StorageTest, SameCallbackCalledAgain)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);

    uint8_t data[] = {0U};
    ::etl::span<uint8_t> dataSpan(data);
    StorageJob::Type::Read::BufferType buf(dataSpan);

    StorageJob job;
    job.init(BLOCKID6, jobDoneCb);
    job.initRead(buf);
    EXPECT_CALL(storageMock, process(_)).WillOnce(Invoke(this, &StorageTest::processCallsCbTwice));
    ASSERT_THROW(storage.process(job), ::estd::assert_exception);
}

TEST_F(StorageTest, SameJobTriggeredAgain)
{
    context.handleExecute();

    uint8_t const data[] = {WRITEVAL, WRITEVAL};
    ::etl::span<uint8_t const> dataSpan(data);
    StorageJob::Type::Write::BufferType buf(dataSpan);

    StorageJob job;
    job.init(BLOCKID3, jobDoneCb);
    job.initWrite(buf);
    // NOTE: the re-triggered job will be queued by QueuingStorage, this only works because
    // it's a different outgoing job object; if it needed to be queued by the mapper instead,
    // then it wouldn't work because the exact same job can't be added to a forward list twice
    // (as a rule of thumb though: don't re-trigger already pending jobs!)
    storage.process(job);
    storage.process(job);

    {
        InSequence const seq;
        EXPECT_CALL(eepMock, read(10U, _, 6U))
            .WillOnce(DoAll(Invoke(this, &StorageTest::eepRead), Return(::bsp::BSP_OK)));
        EXPECT_CALL(eepMock, write(10U, _, 6U))
            .WillOnce(DoAll(Invoke(this, &StorageTest::eepWrite), Return(::bsp::BSP_OK)));
        EXPECT_CALL(eepMock, read(10U, _, 6U))
            .WillOnce(DoAll(Invoke(this, &StorageTest::eepRead), Return(::bsp::BSP_OK)));
        EXPECT_CALL(eepMock, write(10U, _, 6U))
            .WillOnce(DoAll(Invoke(this, &StorageTest::eepWrite), Return(::bsp::BSP_OK)));
    }
    context.execute();

    EXPECT_TRUE(hasSucceeded(BLOCKID3, 2U));
    EXPECT_EQ(eepData[14U], WRITEVAL);
    EXPECT_EQ(eepData[15U], WRITEVAL);
}

TEST_F(StorageTest, SameJobTriggeredFromCallback)
{
    StorageJob::ResultType results[]{StorageJob::Result::Init(), StorageJob::Result::Init()};
    auto cbk = [&results, this](StorageJob& job)
    {
        estd_assert(job.getId() == BLOCKID3);
        static uint8_t idx = 0U;
        results[idx++]     = job.getResult();
        if (idx == 1U)
        {
            // first retrigger identical job
            storage.process(job);
        }
        else
        {
            // then with different id/callback
            job.init(BLOCKID2, jobDoneCb);
            storage.process(job);
        }
    };
    auto cbkFn = StorageJob::JobDoneCallback(cbk);

    context.handleExecute();

    uint8_t const data[] = {WRITEVAL, WRITEVAL};
    ::etl::span<uint8_t const> dataSpan(data);
    StorageJob::Type::Write::BufferType buf(dataSpan);

    StorageJob job;
    job.init(BLOCKID3, cbkFn);
    job.initWrite(buf);
    storage.process(job);

    {
        InSequence const seq;
        EXPECT_CALL(eepMock, read(10U, _, 6U))
            .WillOnce(DoAll(Invoke(this, &StorageTest::eepRead), Return(::bsp::BSP_OK)));
        EXPECT_CALL(eepMock, write(10U, _, 6U))
            .WillOnce(DoAll(Invoke(this, &StorageTest::eepWrite), Return(::bsp::BSP_OK)));
        EXPECT_CALL(eepMock, read(10U, _, 6U))
            .WillOnce(DoAll(Invoke(this, &StorageTest::eepRead), Return(::bsp::BSP_OK)));
        EXPECT_CALL(eepMock, write(10U, _, 6U))
            .WillOnce(DoAll(Invoke(this, &StorageTest::eepWrite), Return(::bsp::BSP_OK)));
    }
    context.execute();

    EXPECT_TRUE(::etl::holds_alternative<StorageJob::Result::Success>(results[0U]));
    EXPECT_TRUE(::etl::holds_alternative<StorageJob::Result::Success>(results[1U]));
    EXPECT_TRUE(hasSucceeded(BLOCKID2));
    EXPECT_EQ(eepData[14U], WRITEVAL);
    EXPECT_EQ(eepData[15U], WRITEVAL);
}

} // anonymous namespace
