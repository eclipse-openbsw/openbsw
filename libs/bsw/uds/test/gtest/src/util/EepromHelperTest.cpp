// Copyright 2024 Accenture.

#include "util/eeprom/EepromHelper.h"

#include "nvstorage/NvStorageMock.h"
#include "nvstorage/NvStorageTypes.h"

#include <async/AsyncMock.h>
#include <async/TestContext.h>
#include <etl/delegate.h>

#include <gmock/gmock.h>

namespace
{
using namespace ::testing;
using namespace ::nvstorage;
using namespace ::eeprom;

struct EepromHelperTest : public ::testing::Test
{
    ::testing::StrictMock<::async::AsyncMock> asyncMock;
    ::async::TestContext udsContext{1U};
    NvStorageMock nvStorageMock;
    EepromHelper eepromHelper{nvStorageMock, udsContext};
    uint8_t buffer[6U];
    uint8_t opReadCount{0}, opWriteCount{0}, reqOk{0}, reqNotOk{0}, reqPending{0};
};

TEST_F(EepromHelperTest, EepromHelperRead)
{
    auto callback =
        [&](NvStorageOperation const op, NvStorageReturnCode const returnCode, NvBlockIdType const)
    {
        ++opReadCount;
        (returnCode == NVSTORAGE_REQ_OK)
            ? ++reqOk
            : ((returnCode == NVSTORAGE_REQ_NOT_OK) ? ++reqNotOk : ++reqPending);
    };
    EepromJobFinishedNotification cbk = ::etl::delegate::create(callback);

    udsContext.handleExecute();
    EXPECT_CALL(nvStorageMock, blockRead(_, _, _, _))
        .Times(3)
        .WillOnce(DoAll(
            InvokeArgument<2>(
                NvStorageOperation::NVSTORAGE_READ, NvStorageReturnCode::NVSTORAGE_REQ_OK, 0),
            Return(NvStorageReturnCode::NVSTORAGE_REQ_OK)))
        .WillOnce(DoAll(
            InvokeArgument<2>(
                NvStorageOperation::NVSTORAGE_READ, NvStorageReturnCode::NVSTORAGE_REQ_NOT_OK, 0),
            Return(NvStorageReturnCode::NVSTORAGE_REQ_NOT_OK)))
        .WillOnce(DoAll(
            InvokeArgument<2>(
                NvStorageOperation::NVSTORAGE_READ, NvStorageReturnCode::NVSTORAGE_REQ_PENDING, 0),
            Return(NvStorageReturnCode::NVSTORAGE_REQ_PENDING)));

    EXPECT_TRUE(eepromHelper.read(0, buffer, 4, cbk));
    udsContext.execute();
    EXPECT_FALSE(eepromHelper.read(0, buffer, 3, cbk));
    udsContext.execute();
    EXPECT_TRUE(eepromHelper.read(0, buffer, 3, cbk));
    udsContext.execute();
    EXPECT_EQ(3, opReadCount);
    EXPECT_TRUE((reqOk == 1) && (reqNotOk == 1) && (reqPending == 1));
}

TEST_F(EepromHelperTest, EepromHelperWrite)
{
    auto callback =
        [&](NvStorageOperation const op, NvStorageReturnCode const returnCode, NvBlockIdType const)
    {
        ++opWriteCount;
        (returnCode == NVSTORAGE_REQ_OK)
            ? ++reqOk
            : ((returnCode == NVSTORAGE_REQ_NOT_OK) ? ++reqNotOk : ++reqPending);
    };
    EepromJobFinishedNotification cbk = ::etl::delegate::create(callback);

    udsContext.handleExecute();
    EXPECT_CALL(nvStorageMock, blockWrite(_, _, _, _))
        .Times(3)
        .WillOnce(DoAll(
            InvokeArgument<3>(
                NvStorageOperation::NVSTORAGE_WRITE, NvStorageReturnCode::NVSTORAGE_REQ_OK, 0),
            Return(NvStorageReturnCode::NVSTORAGE_REQ_OK)))
        .WillOnce(DoAll(
            InvokeArgument<3>(
                NvStorageOperation::NVSTORAGE_WRITE, NvStorageReturnCode::NVSTORAGE_REQ_NOT_OK, 0),
            Return(NvStorageReturnCode::NVSTORAGE_REQ_NOT_OK)))
        .WillOnce(DoAll(
            InvokeArgument<3>(
                NvStorageOperation::NVSTORAGE_WRITE, NvStorageReturnCode::NVSTORAGE_REQ_PENDING, 0),
            Return(NvStorageReturnCode::NVSTORAGE_REQ_PENDING)));

    EXPECT_TRUE(eepromHelper.write(0, buffer, 4, cbk));
    udsContext.execute();
    EXPECT_FALSE(eepromHelper.write(0, buffer, 5, cbk));
    udsContext.execute();
    EXPECT_TRUE(eepromHelper.write(0, buffer, 5, cbk));
    udsContext.execute();
    EXPECT_EQ(3, opWriteCount);
    EXPECT_TRUE((reqOk == 1) && (reqNotOk == 1) && (reqPending == 1));
}
} // namespace
