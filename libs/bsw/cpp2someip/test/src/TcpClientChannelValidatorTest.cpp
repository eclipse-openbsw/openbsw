/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/TcpClientChannelValidator.h"

#include "someip/NetworkMock.h"
#include "someip/NetworkResourceMock.h"
#include "someip/SomeIpConstants.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace
{
using namespace ::testing;
using namespace ::someip;

class TcpClientChannelValidatorTest : public ::testing::Test
{
public:
    TcpClientChannelValidatorTest() { _proxy.incRefCounter(); }

    void TearDown() override { _proxy.decRefCounter(); }

protected:
    NiceMock<::someip::NetworkResourceMock> _proxy;
    NiceMock<::someip::NetworkMock> _network;
};

/**
 * Test construction of TcpClientChannelValidatorTest.
 */
TEST_F(TcpClientChannelValidatorTest, test_construction)
{
    ::someip::TcpClientChannelValidator validator(_network, false);
}

/**
 * Test construction of TcpClientChannelValidator::CachedValidator.
 */
TEST_F(TcpClientChannelValidatorTest, test_construction_CachedValidator)
{
    ::someip::TcpClientChannelValidator validator(_network, false);
    ::someip::TcpClientChannelValidator::CachedValidator cv(validator);
}

/**
 * This test verifies if isChannelEstablished works as expected, meaning:
 * It will return true, if the NetworkChannel is valid and a connection
 * is established. Otherwise it will return false.
 */
TEST_F(TcpClientChannelValidatorTest, CachedValidator_isChannelEstablished)
{
    ::ip::IPAddress remoteAddr = ::ip::make_ip4(192U, 0U, 2U, 1U);
    uint16_t remotePort        = 10U;
    uint16_t localPort         = 11U;
    ::ip::IPEndpoint remoteEp(remoteAddr, remotePort);
    ::someip::TcpClientChannelValidator validator(_network, false);

    // NetworkChannel is valid, connection is not established
    {
        ::someip::TcpClientChannelValidator::CachedValidator cv(validator);

        EXPECT_CALL(
            _network,
            getRpcChannel(
                localPort, remoteEp, static_cast<uint8_t>(::someip::proto::SD_L4_PROTO_TCP)))
            .WillOnce(Return(::etl::optional<::someip::NetworkChannel>(
                ::someip::NetworkChannel(_proxy, remoteEp, false))));

        EXPECT_CALL(_proxy, isConnected()).WillOnce(Return(false));

        bool result = cv.isChannelEstablished(remoteEp, localPort);
        EXPECT_FALSE(result);

        _proxy.decRefCounter();
        _proxy.decRefCounter();
    }

    // NetworkChannel is valid, connection is established
    {
        ::someip::TcpClientChannelValidator::CachedValidator cv(validator);

        EXPECT_CALL(
            _network,
            getRpcChannel(
                localPort, remoteEp, static_cast<uint8_t>(::someip::proto::SD_L4_PROTO_TCP)))
            .WillOnce(Return(::etl::optional<::someip::NetworkChannel>(
                ::someip::NetworkChannel(_proxy, remoteEp, false))));
        EXPECT_CALL(_proxy, isConnected()).WillOnce(Return(true));

        bool result = cv.isChannelEstablished(remoteEp, localPort);
        EXPECT_TRUE(result);
        _proxy.decRefCounter();
        _proxy.decRefCounter();
    }

    // NetworkChannel is not valid, connection status won't be evaluated
    {
        ::someip::TcpClientChannelValidator::CachedValidator cv(validator);

        EXPECT_CALL(
            _network,
            getRpcChannel(
                localPort, remoteEp, static_cast<uint8_t>(::someip::proto::SD_L4_PROTO_TCP)))
            .WillOnce(Return(::etl::optional<::someip::NetworkChannel>()));

        bool result = cv.isChannelEstablished(remoteEp, localPort);
        EXPECT_FALSE(result);
    }
}

TEST_F(TcpClientChannelValidatorTest, CachedValidator_checkClientChannel)
{
    ::ip::IPAddress remoteAddr  = ::ip::make_ip4(192U, 0U, 2U, 1U);
    ::ip::IPAddress remoteAddr2 = ::ip::make_ip4(192U, 0U, 2U, 2U);
    uint16_t remotePort         = 10U;
    uint16_t localPort          = 11U;
    ::ip::IPEndpoint remoteEp(remoteAddr, remotePort);
    ::ip::IPEndpoint remoteEp2(remoteAddr2, remotePort);
    ::someip::TcpClientChannelValidator validator(_network, true);
    StrictMock<::someip::NetworkResourceMock> networkResourseMock;
    // suppose the resource is used
    networkResourseMock.incRefCounter();

    {
        ::someip::TcpClientChannelValidator::CachedValidator cv(validator);

        EXPECT_CALL(
            _network,
            getRpcChannel(
                localPort, remoteEp, static_cast<uint8_t>(::someip::proto::SD_L4_PROTO_TCP)))
            .WillOnce(Return(::etl::optional<::someip::NetworkChannel>(
                ::someip::NetworkChannel(networkResourseMock, remoteEp, false, false))));

        cv.checkClientChannel(remoteEp, localPort);
        cv.checkClientChannel(remoteEp, localPort);

        Mock::VerifyAndClearExpectations(&_network);
        Mock::VerifyAndClearExpectations(&networkResourseMock);
    }
    {
        ::someip::TcpClientChannelValidator::CachedValidator cv(validator);

        EXPECT_CALL(
            _network,
            getRpcChannel(
                localPort, remoteEp, static_cast<uint8_t>(::someip::proto::SD_L4_PROTO_TCP)))
            .WillOnce(Return(::etl::optional<::someip::NetworkChannel>(
                ::someip::NetworkChannel(networkResourseMock, remoteEp, false, true))));

        EXPECT_CALL(networkResourseMock, isOpen()).WillOnce(Return(true));
        EXPECT_CALL(networkResourseMock, isConnected()).WillOnce(Return(true));
        EXPECT_CALL(networkResourseMock, send(::someip::SomeIpConstants::HEADER_LENGTH))
            .WillOnce(Return(true));

        cv.checkClientChannel(remoteEp, localPort);
        cv.checkClientChannel(remoteEp, localPort);

        Mock::VerifyAndClearExpectations(&_network);
        Mock::VerifyAndClearExpectations(&networkResourseMock);
    }
    {
        ::someip::TcpClientChannelValidator::CachedValidator cv(validator);

        EXPECT_CALL(
            _network,
            getRpcChannel(
                localPort, remoteEp, static_cast<uint8_t>(::someip::proto::SD_L4_PROTO_TCP)))
            .WillOnce(Return(::etl::optional<::someip::NetworkChannel>(
                ::someip::NetworkChannel(networkResourseMock, remoteEp, false, true))));

        EXPECT_CALL(networkResourseMock, isOpen()).WillOnce(Return(true));
        EXPECT_CALL(networkResourseMock, isConnected()).WillOnce(Return(true));
        EXPECT_CALL(networkResourseMock, send(::someip::SomeIpConstants::HEADER_LENGTH))
            .WillOnce(Return(true));

        cv.checkClientChannel(remoteEp, localPort);

        EXPECT_CALL(
            _network,
            getRpcChannel(
                localPort, remoteEp2, static_cast<uint8_t>(::someip::proto::SD_L4_PROTO_TCP)))
            .WillOnce(Return(::etl::optional<::someip::NetworkChannel>(
                ::someip::NetworkChannel(networkResourseMock, remoteEp2, false, true))));

        EXPECT_CALL(networkResourseMock, isOpen()).WillOnce(Return(true));
        EXPECT_CALL(networkResourseMock, isConnected()).WillOnce(Return(true));
        EXPECT_CALL(networkResourseMock, send(::someip::SomeIpConstants::HEADER_LENGTH))
            .WillOnce(Return(true));
        cv.checkClientChannel(remoteEp2, localPort);

        Mock::VerifyAndClearExpectations(&_network);
        Mock::VerifyAndClearExpectations(&networkResourseMock);
    }
}

} // namespace
