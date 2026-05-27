/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "doip/common/DoIpTestHelpers.h"

#include <etl/span.h>

using namespace ::testing;
using namespace ::tcp::test;

namespace doip
{
namespace test
{
DoIpExpectSendHelper& DoIpExpectSendHelper::send(uint32_t size)
{
    EXPECT_TRUE(size <= _unsentSize);
    EXPECT_CALL(_socketMock, send(ElementsAreArray(_message, size)))
        .InSequence(_seq)
        .WillOnce(Return(::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK));
    _message += size;
    _unsentSize -= size;
    return *this;
}

DoIpExpectSendHelper& DoIpExpectSendHelper::flush()
{
    EXPECT_CALL(_socketMock, flush())
        .InSequence(_seq)
        .WillOnce(Return(::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK));
    return *this;
}

DoIpExpectSendHelper& DoIpExpectSendHelper::verify(::async::TestContext& testContext)
{
    EXPECT_THAT(_socketMock.getSendNotificationListener(), NotNull());
    testContext.expireAndExecute();
    Mock::VerifyAndClear(&_socketMock);
    return *this;
}

void DoIpExpectSendHelper::dataSentAndVerify(::async::TestContext& testContext)
{
    _socketMock.getSendNotificationListener()->dataSent(
        _size, ::tcp::IDataSendNotificationListener::SendResult::DATA_SENT);
    testContext.expireAndExecute();
    Mock::VerifyAndClear(&_socketMock);
}

DoIpExpectReadHelper& DoIpExpectReadHelper::read(uint32_t size)
{
    EXPECT_TRUE(size <= _unreadSize);
    EXPECT_CALL(_socketMock, read(NotNull(), size))
        .InSequence(_seq)
        .WillOnce(Invoke(ReadBytesFrom(::etl::span<uint8_t const>(_message, size))));
    _message += size;
    _unreadSize -= size;
    return *this;
}

DoIpExpectReadHelper& DoIpExpectReadHelper::skip(uint32_t size)
{
    EXPECT_TRUE(size <= _unreadSize);
    EXPECT_CALL(_socketMock, read(nullptr, size)).InSequence(_seq).WillOnce(Return(size));
    _message += size;
    _unreadSize -= size;
    return *this;
}

void DoIpExpectReadHelper::dataReceivedAndVerify(::async::TestContext& testContext)
{
    EXPECT_THAT(_socketMock.getDataListener(), NotNull());
    _socketMock.getDataListener()->dataReceived(_size);
    testContext.expireAndExecute();
    Mock::VerifyAndClear(&_socketMock);
}

} // namespace test
} // namespace doip
