// Copyright 2025 Accenture.

#include <async/Async.h>
#include <async/AsyncMock.h>
#include <async/TestContext.h>
#include <bsp/eeprom/EepromDriverMock.h>
#include <etl/array.h>
#include <etl/memory.h>
#include <etl/span.h>
#include <storage/EepStorage.h>
#include <storage/FeeStorage.h>
#include <storage/IStorageMock.h>
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
static uint32_t const BLOCKID5      = 104U; // this one will be unused
static uint32_t const BLOCKID6      = 105U;
static uint32_t const BLOCKID7      = 106U;
static uint32_t const BLOCKID8      = 107U;
static uint32_t const BLOCKID9      = 108U;
static uint8_t const EEPSTORAGEIDX  = 0U;
static uint8_t const FEESTORAGEIDX  = 1U;
static uint8_t const MOCKSTORAGEIDX = 2U;

static constexpr ::storage::MappingConfig MAPPING_CONFIG[] = {
    {BLOCKID1, 0U, EEPSTORAGEIDX},
    {BLOCKID2, 0U, FEESTORAGEIDX},
    {BLOCKID3, 1U, EEPSTORAGEIDX},
    {BLOCKID4, 2U, EEPSTORAGEIDX},
    {BLOCKID6, 3U, EEPSTORAGEIDX},
    {BLOCKID7, 0U, MOCKSTORAGEIDX},
    {BLOCKID8, 4U, EEPSTORAGEIDX},
    {BLOCKID9, 0U, 3U}, // storage index exceeds the limit given for MappingStorage
};

static constexpr ::storage::EepBlockConfig EEP_BLOCK_CONFIG[] = {
    {0U /* address */, 2U /* size (without 4-byte header) */, true /* error detection */},
    {10U, 3U, true},
    {20U, 4U, false},
    {30U, 4U, true},
    {38U, 5U, true}, // invalid data size (bigger than the limit given for eepStorage)
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
        (void)::etl::mem_set(jobResults[0U], sizeof(jobResults), static_cast<uint8_t>(0U));
        eepData.fill(EEP_INITVAL1);
        // for the sake of variety, fill BLOCKID1 for different init values
        (void)::etl::mem_set(eepData.data(), 6U, EEP_INITVAL2);
        // store correct, precalculated checksums for reading that match the default data
        eepData[0U]  = 0x99U; // BLOCKID1
        eepData[1U]  = 0xCFU;
        eepData[10U] = 0x20U; // BLOCKID3
        eepData[11U] = 0x0BU;
        eepData[30U] = 0x51U; // BLOCKID6
        eepData[31U] = 0xD5U;
        // used sizes
        eepData[2U]  = 0U;
        eepData[3U]  = 2U;
        eepData[12U] = 0U;
        eepData[13U] = 3U;
        eepData[32U] = 0U;
        eepData[33U] = 1U; // NOTE: smaller than the defined dataSize
    }

    void eepRead(uint32_t address, uint8_t* dst, uint32_t length)
    {
        estd_assert((address + length) <= sizeof(eepData));
        (void)::etl::mem_copy(&(eepData[address]), length, dst);
    }

    void eepWrite(uint32_t address, uint8_t const* src, uint32_t length)
    {
        estd_assert((address + length) <= sizeof(eepData));
        (void)::etl::mem_copy(src, length, &(eepData[address]));
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
        4U /* max data size */>
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
    ::etl::array<uint8_t, EEP_LAYOUT_SIZE> eepData;

    static_assert(BLOCKID9 > BLOCKID1, "block IDs must be defined in ascending order");
    uint8_t jobResults[(BLOCKID9 - BLOCKID1 + 1)][4U /* number of result kinds */];
    StorageJob::JobDoneCallback const jobDoneCb;

    uint8_t const EEP_INITVAL1 = 111U;
    uint8_t const EEP_INITVAL2 = 255U;
    uint8_t const FEE_INITVAL  = 222U;
    uint8_t const WRITEVAL     = 90U;
    uint8_t const WRITEVAL2    = 100U;
};

TEST_F(StorageTest, MultipleJobsAtOnce)
{
    context.handleExecute();

    uint8_t data[] = {0U, 0U};
    StorageJob::Type::Read::BufferType buf(data);

    StorageJob job;
    job.init(BLOCKID1, jobDoneCb);
    job.initRead(buf);
    storage.process(job);

    uint8_t data2[] = {0U};
    StorageJob::Type::Read::BufferType buf2(data2);

    StorageJob job2;
    job2.init(BLOCKID2, jobDoneCb);
    job2.initRead(buf2);
    storage.process(job2);

    uint8_t const data3[] = {0U};
    StorageJob::Type::Write::BufferType buf3(data3);

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
    EXPECT_EQ(job.getRead().getReadSize(), buf.getBuffer().size());
    EXPECT_EQ(job2.getRead().getReadSize(), buf2.getBuffer().size());
    EXPECT_EQ(data[0U], EEP_INITVAL2);
    EXPECT_EQ(data[1U], EEP_INITVAL2);
    EXPECT_EQ(data2[0U], FEE_INITVAL);
}

TEST_F(StorageTest, ReadWithChecksumIntoSmallerBuffer)
{
    context.handleExecute();

    uint8_t data[] = {0U};
    StorageJob::Type::Read::BufferType buf(data);

    StorageJob job;
    job.init(BLOCKID3, jobDoneCb);
    job.initRead(buf);
    storage.process(job);

    EXPECT_CALL(eepMock, read(10U, _, 7U))
        .WillOnce(DoAll(Invoke(this, &StorageTest::eepRead), Return(::bsp::BSP_OK)));
    context.execute();

    EXPECT_TRUE(hasSucceeded(BLOCKID3));
    EXPECT_EQ(job.getRead().getReadSize(), buf.getBuffer().size());
    EXPECT_EQ(data[0U], EEP_INITVAL1);
}

TEST_F(StorageTest, ReadWithChecksumIntoBiggerBuffer)
{
    context.handleExecute();

    uint8_t data[] = {0U, 0U};
    StorageJob::Type::Read::BufferType buf(data);

    StorageJob job;
    job.init(BLOCKID6, jobDoneCb);
    job.initRead(buf);
    storage.process(job);

    EXPECT_CALL(eepMock, read(30U, _, 8U))
        .WillOnce(DoAll(Invoke(this, &StorageTest::eepRead), Return(::bsp::BSP_OK)));
    context.execute();

    EXPECT_TRUE(hasSucceeded(BLOCKID6));
    EXPECT_EQ(job.getRead().getReadSize(), 1U);
    EXPECT_EQ(data[0U], EEP_INITVAL1);
}

TEST_F(StorageTest, WriteWithChecksumFromSmallerBuffer)
{
    context.handleExecute();

    uint8_t const data[] = {WRITEVAL};
    StorageJob::Type::Write::BufferType buf(data);

    StorageJob job;
    job.init(BLOCKID3, jobDoneCb);
    job.initWrite(buf);
    storage.process(job);

    {
        InSequence const seq;
        EXPECT_CALL(eepMock, read(10U, _, 7U))
            .WillOnce(DoAll(Invoke(this, &StorageTest::eepRead), Return(::bsp::BSP_OK)));
        EXPECT_CALL(eepMock, write(10U, _, 5U))
            .WillOnce(DoAll(Invoke(this, &StorageTest::eepWrite), Return(::bsp::BSP_OK)));
    }
    context.execute();

    EXPECT_TRUE(hasSucceeded(BLOCKID3));
    EXPECT_EQ(eepData[10U], 0x0EU); // verify checksum
    EXPECT_EQ(eepData[11U], 0x5EU);
    EXPECT_EQ(eepData[12U], 0U);
    EXPECT_EQ(eepData[13U], 3U);       // size should still be 3
    EXPECT_EQ(eepData[14U], WRITEVAL); // only the 1st data byte has been written
    EXPECT_EQ(eepData[15U], EEP_INITVAL1);
}

TEST_F(StorageTest, ReadIntoTwoBuffers)
{
    context.handleExecute();

    uint8_t data[] = {0U, 0U};
    StorageJob::Type::Read::BufferType buf(data);
    uint8_t data2[] = {0U, 0U};
    StorageJob::Type::Read::BufferType buf2(data2);
    buf.setNext(&buf2);

    StorageJob job;
    job.init(BLOCKID4, jobDoneCb);
    job.initRead(buf);
    storage.process(job);

    EXPECT_CALL(eepMock, read(20U, _, 4U))
        .WillOnce(DoAll(Invoke(this, &StorageTest::eepRead), Return(::bsp::BSP_OK)));
    context.execute();

    EXPECT_TRUE(hasSucceeded(BLOCKID4));
    EXPECT_EQ(job.getRead().getReadSize(), (buf.getBuffer().size() + buf2.getBuffer().size()));
    EXPECT_EQ(data[0U], EEP_INITVAL1);
    EXPECT_EQ(data[1U], EEP_INITVAL1);
    EXPECT_EQ(data2[0U], EEP_INITVAL1);
    EXPECT_EQ(data2[1U], EEP_INITVAL1);
}

TEST_F(StorageTest, WriteFromTwoBuffers)
{
    context.handleExecute();

    uint8_t const data[] = {WRITEVAL, WRITEVAL};
    StorageJob::Type::Write::BufferType buf(data);
    uint8_t const data2[] = {WRITEVAL2, WRITEVAL2};
    StorageJob::Type::Write::BufferType buf2(data2);
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
    StorageJob::Type::Read::BufferType buf(data);

    StorageJob job;
    job.init(BLOCKID3, jobDoneCb);
    job.initRead(buf, 1U);
    storage.process(job);

    EXPECT_CALL(eepMock, read(10U, _, 7U))
        .WillOnce(DoAll(Invoke(this, &StorageTest::eepRead), Return(::bsp::BSP_OK)));
    ++eepData[15U];       // change the 2nd data byte so that we can distinguish it from the 1st
    eepData[10U] = 0x33U; // update checksum
    eepData[11U] = 0x46U;
    context.execute();

    EXPECT_TRUE(hasSucceeded(BLOCKID3));
    EXPECT_EQ(job.getRead().getReadSize(), buf.getBuffer().size());
    EXPECT_EQ(data[0U], (EEP_INITVAL1 + 1U));
}

TEST_F(StorageTest, WriteToOffset)
{
    context.handleExecute();

    uint8_t const data[] = {WRITEVAL};
    StorageJob::Type::Write::BufferType buf(data);

    StorageJob job;
    job.init(BLOCKID3, jobDoneCb);
    job.initWrite(buf, 1U);
    storage.process(job);

    {
        InSequence const seq;
        EXPECT_CALL(eepMock, read(10U, _, 7U))
            .WillOnce(DoAll(Invoke(this, &StorageTest::eepRead), Return(::bsp::BSP_OK)));
        EXPECT_CALL(eepMock, write(10U, _, 6U))
            .WillOnce(DoAll(Invoke(this, &StorageTest::eepWrite), Return(::bsp::BSP_OK)));
    }
    context.execute();

    EXPECT_TRUE(hasSucceeded(BLOCKID3));
    EXPECT_EQ(eepData[10U], 0xDAU); // verify checksum
    EXPECT_EQ(eepData[11U], 0x6BU);
    EXPECT_EQ(eepData[12U], 0U); // verify size
    EXPECT_EQ(eepData[13U], 3U);
    EXPECT_EQ(eepData[14U], EEP_INITVAL1);
    EXPECT_EQ(eepData[15U], WRITEVAL);
    EXPECT_EQ(eepData[16U], EEP_INITVAL1);
}

TEST_F(StorageTest, WriteToOffsetWithoutExistingData)
{
    context.handleExecute();

    uint8_t const data[] = {WRITEVAL};
    StorageJob::Type::Write::BufferType buf(data);

    StorageJob job;
    job.init(BLOCKID3, jobDoneCb);
    job.initWrite(buf, 1U);
    storage.process(job);

    {
        InSequence const seq;
        EXPECT_CALL(eepMock, read(10U, _, 7U))
            .WillOnce(DoAll(Invoke(this, &StorageTest::eepRead), Return(::bsp::BSP_OK)));
        EXPECT_CALL(eepMock, write(10U, _, 6U))
            .WillOnce(DoAll(Invoke(this, &StorageTest::eepWrite), Return(::bsp::BSP_OK)));
    }
    ++eepData[10U]; // invalidate existing checksum
    context.execute();

    EXPECT_TRUE(hasSucceeded(BLOCKID3));
    EXPECT_EQ(eepData[10U], 0x7FU); // verify checksum
    EXPECT_EQ(eepData[11U], 0x7FU);
    EXPECT_EQ(eepData[12U], 0U); // verify size
    EXPECT_EQ(eepData[13U], 2U);
    EXPECT_EQ(eepData[14U], 0U); // 1st data byte initialized to default
    EXPECT_EQ(eepData[15U], WRITEVAL);
    EXPECT_EQ(eepData[16U], EEP_INITVAL1);
}

TEST_F(StorageTest, ReadError)
{
    context.handleExecute();

    uint8_t data[] = {0U};
    StorageJob::Type::Read::BufferType buf(data);

    StorageJob job;
    job.init(BLOCKID3, jobDoneCb);
    job.initRead(buf);
    storage.process(job);

    EXPECT_CALL(eepMock, read(10U, _, 7U))
        .WillOnce(DoAll(Invoke(this, &StorageTest::eepRead), Return(::bsp::BSP_ERROR)));
    context.execute();

    EXPECT_TRUE(hasFailed(BLOCKID3));
    EXPECT_EQ(job.getRead().getReadSize(), 0U);
}

TEST_F(StorageTest, WriteError)
{
    context.handleExecute();

    uint8_t const data[] = {0U};
    StorageJob::Type::Write::BufferType buf(data);

    StorageJob job;
    job.init(BLOCKID3, jobDoneCb);
    job.initWrite(buf);
    storage.process(job);

    EXPECT_CALL(eepMock, read(10U, _, 7U))
        .WillOnce(DoAll(Invoke(this, &StorageTest::eepRead), Return(::bsp::BSP_ERROR)));
    context.execute();
    EXPECT_TRUE(hasFailed(BLOCKID3));

    storage.process(job);

    {
        InSequence const seq;
        EXPECT_CALL(eepMock, read(10U, _, 7U))
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
    StorageJob::Type::Read::BufferType buf(data);

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
    StorageJob::Type::Read::BufferType buf(data);

    StorageJob job;
    job.init(BLOCKID3, jobDoneCb);
    job.initRead(buf, 3U);
    storage.process(job);

    EXPECT_CALL(eepMock, read(10U, _, 7U))
        .WillOnce(DoAll(Invoke(this, &StorageTest::eepRead), Return(::bsp::BSP_OK)));
    context.execute();
    EXPECT_TRUE(hasSucceeded(BLOCKID3));
    EXPECT_EQ(job.getRead().getReadSize(), 0U);

    job.initRead(buf, 4U);
    storage.process(job);

    EXPECT_CALL(eepMock, read(10U, _, 7U))
        .WillOnce(DoAll(Invoke(this, &StorageTest::eepRead), Return(::bsp::BSP_OK)));
    context.execute();
    EXPECT_TRUE(hasSucceeded(BLOCKID3, 2U));
    EXPECT_EQ(job.getRead().getReadSize(), 0U);
}

TEST_F(StorageTest, InvalidWriteSize)
{
    context.handleExecute();

    uint8_t const data[] = {WRITEVAL, WRITEVAL, WRITEVAL};
    StorageJob::Type::Write::BufferType buf(data);

    StorageJob job;
    job.init(BLOCKID3, jobDoneCb);
    job.initWrite(buf, 1U);
    storage.process(job);

    EXPECT_CALL(eepMock, read(10U, _, 7U))
        .WillOnce(DoAll(Invoke(this, &StorageTest::eepRead), Return(::bsp::BSP_OK)));
    context.execute();
    EXPECT_TRUE(hasFailed(BLOCKID3));
}

TEST_F(StorageTest, EmptyWrite)
{
    context.handleExecute();

    ::etl::span<uint8_t const> emptySpan;
    StorageJob::Type::Write::BufferType buf(emptySpan);

    StorageJob job;
    job.init(BLOCKID3, jobDoneCb);
    job.initWrite(buf, 1U);
    storage.process(job);

    EXPECT_CALL(eepMock, read(10U, _, 7U))
        .WillOnce(DoAll(Invoke(this, &StorageTest::eepRead), Return(::bsp::BSP_OK)));
    context.execute();
    EXPECT_TRUE(hasFailed(BLOCKID3));
}

TEST_F(StorageTest, InvalidWriteOffset)
{
    context.handleExecute();

    uint8_t const data[] = {WRITEVAL};
    StorageJob::Type::Write::BufferType buf(data);

    StorageJob job;
    job.init(BLOCKID3, jobDoneCb);
    job.initWrite(buf, 3U);
    storage.process(job);
    context.execute();
    EXPECT_TRUE(hasFailed(BLOCKID3));

    job.initWrite(buf, 4U);
    storage.process(job);
    context.execute();
    EXPECT_TRUE(hasFailed(BLOCKID3, 2U));
}

TEST_F(StorageTest, InvalidBlockId)
{
    context.handleExecute();

    StorageJob::ResultType result = StorageJob::Result::Init();
    auto cbk                      = [&result](StorageJob& job) { result = job.getResult(); };

    uint8_t data[] = {0U};
    StorageJob::Type::Read::BufferType buf(data);

    StorageJob job;
    // try an id smaller than the smallest known one
    job.init(99U, StorageJob::JobDoneCallback(cbk));
    job.initRead(buf);
    storage.process(job);
    context.execute();
    EXPECT_TRUE(::etl::holds_alternative<StorageJob::Result::Error>(result));
    EXPECT_TRUE(job.hasResult<StorageJob::Result::Error>());
    result = StorageJob::Result::Init();

    // unknown id in the middle
    job.init(BLOCKID5, StorageJob::JobDoneCallback(cbk));
    storage.process(job);
    context.execute();
    EXPECT_TRUE(::etl::holds_alternative<StorageJob::Result::Error>(result));
    result = StorageJob::Result::Init();

    // id bigger than the biggest known one
    job.init(109U, StorageJob::JobDoneCallback(cbk));
    storage.process(job);
    context.execute();
    EXPECT_TRUE(::etl::holds_alternative<StorageJob::Result::Error>(result));
    result = StorageJob::Result::Init();

    // try calling eepStorage directly
    job.init(5U, StorageJob::JobDoneCallback(cbk));
    eepStorage.process(job);
    EXPECT_TRUE(::etl::holds_alternative<StorageJob::Result::Error>(result));
}

TEST_F(StorageTest, InvalidDataSize)
{
    context.handleExecute();

    uint8_t data[] = {0U};
    StorageJob::Type::Read::BufferType buf(data);

    StorageJob job;
    job.init(BLOCKID8, jobDoneCb);
    job.initRead(buf);
    storage.process(job);
    context.execute();
    EXPECT_TRUE(hasFailed(BLOCKID8));
}

TEST_F(StorageTest, InvalidStorageIdx)
{
    context.handleExecute();

    uint8_t data[] = {0U};
    StorageJob::Type::Read::BufferType buf(data);

    StorageJob job;
    job.init(BLOCKID9, jobDoneCb);
    job.initRead(buf);
    storage.process(job);
    context.execute();
    EXPECT_TRUE(hasFailed(BLOCKID9));
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
    StorageJob::Type::Read::BufferType buf(data);

    StorageJob job;
    job.init(BLOCKID7, jobDoneCb);
    job.initRead(buf);
    EXPECT_CALL(storageMock, process(_)).WillOnce(Invoke(this, &StorageTest::processCallsCbTwice));
    ASSERT_THROW(storage.process(job), ::estd::assert_exception);
}

TEST_F(StorageTest, SameJobTriggeredAgain)
{
    context.handleExecute();

    uint8_t const data[] = {WRITEVAL, WRITEVAL};
    StorageJob::Type::Write::BufferType buf(data);

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
        EXPECT_CALL(eepMock, read(10U, _, 7U))
            .WillOnce(DoAll(Invoke(this, &StorageTest::eepRead), Return(::bsp::BSP_OK)));
        EXPECT_CALL(eepMock, write(10U, _, 6U))
            .WillOnce(DoAll(Invoke(this, &StorageTest::eepWrite), Return(::bsp::BSP_OK)));
        EXPECT_CALL(eepMock, read(10U, _, 7U))
            .WillOnce(DoAll(Invoke(this, &StorageTest::eepRead), Return(::bsp::BSP_OK)));
        EXPECT_CALL(eepMock, write(10U, _, 6U))
            .WillOnce(DoAll(Invoke(this, &StorageTest::eepWrite), Return(::bsp::BSP_OK)));
    }
    context.execute();

    EXPECT_TRUE(hasSucceeded(BLOCKID3, 2U));
    EXPECT_EQ(eepData[14U], WRITEVAL);
    EXPECT_EQ(eepData[15U], WRITEVAL);
    EXPECT_EQ(eepData[16U], EEP_INITVAL1);
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

    uint8_t const data[] = {WRITEVAL, WRITEVAL, WRITEVAL};
    StorageJob::Type::Write::BufferType buf(data);

    StorageJob job;
    job.init(BLOCKID3, cbkFn);
    job.initWrite(buf);
    storage.process(job);

    {
        InSequence const seq;
        EXPECT_CALL(eepMock, read(10U, _, 7U))
            .WillOnce(DoAll(Invoke(this, &StorageTest::eepRead), Return(::bsp::BSP_OK)));
        EXPECT_CALL(eepMock, write(10U, _, 7U))
            .WillOnce(DoAll(Invoke(this, &StorageTest::eepWrite), Return(::bsp::BSP_OK)));
        EXPECT_CALL(eepMock, read(10U, _, 7U))
            .WillOnce(DoAll(Invoke(this, &StorageTest::eepRead), Return(::bsp::BSP_OK)));
        EXPECT_CALL(eepMock, write(10U, _, 7U))
            .WillOnce(DoAll(Invoke(this, &StorageTest::eepWrite), Return(::bsp::BSP_OK)));
    }
    context.execute();

    EXPECT_TRUE(::etl::holds_alternative<StorageJob::Result::Success>(results[0U]));
    EXPECT_TRUE(::etl::holds_alternative<StorageJob::Result::Success>(results[1U]));
    EXPECT_TRUE(hasSucceeded(BLOCKID2));
    EXPECT_EQ(eepData[14U], WRITEVAL);
    EXPECT_EQ(eepData[15U], WRITEVAL);
    EXPECT_EQ(eepData[16U], WRITEVAL);
}

} // anonymous namespace
