/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "TestConstants.h"
#include "someip/NetworkListenerMock.h"
#include "someip/UdpConfig.h"

#include <udp/socket/AbstractDatagramSocketMock.h>

#include <etl/span.h>

#include <gmock/gmock.h>

namespace
{
using namespace ::testing;
using namespace ::someip;
using namespace ::test;

struct UdpProxyConfigInstance : UdpProxyConfig
{
    UdpProxyConfigInstance() : UdpProxyConfig(proxyArray, serverProxyList, portList)
    {
        for (uint8_t i = 0U; i < PROXIES_SIZE; ++i)
        {
            UdpProxy& proxy = proxyArray[i];
            proxy.setSocket(socketArray[i]);
            proxy.setInputBuffer(inputBuffer);
            proxy.setOutputBuffer(outputBuffer);
        }
    }

    static constexpr size_t PROXIES_SIZE = 2U;

    uint8_t inputBuffer[1500U];
    uint8_t outputBuffer[1500U];
    ::udp::AbstractDatagramSocketMock socketArray[PROXIES_SIZE];
    UdpProxy proxyArray[PROXIES_SIZE];

    ::etl::vector<UdpProxy*, PROXIES_SIZE> serverProxyList;
    ::etl::vector<uint16_t, PROXIES_SIZE> portList;
};

constexpr size_t UdpProxyConfigInstance::PROXIES_SIZE;

struct UdpProxyConfigTest : Test
{
    UdpProxyConfigInstance _config;
};

TEST_F(UdpProxyConfigTest, ctor)
{
    EXPECT_EQ(0U, _config.serverProxyList.size());
    EXPECT_EQ(UdpProxyConfigInstance::PROXIES_SIZE, _config.serverProxyList.max_size());
    EXPECT_EQ(0U, _config.portList.size());
    EXPECT_EQ(UdpProxyConfigInstance::PROXIES_SIZE, _config.portList.max_size());

    {
        uint8_t inputBuffer[1500U];
        uint8_t outputBuffer[1500U];
        ::someip::internal::UdpProxyResources<
            ::udp::AbstractDatagramSocketMock,
            UdpProxyConfigInstance::PROXIES_SIZE>
            configWithResources(inputBuffer, outputBuffer);

        EXPECT_EQ(UdpProxyConfigInstance::PROXIES_SIZE, configWithResources.getSize());

        for (size_t i = 0U; i < UdpProxyConfigInstance::PROXIES_SIZE; i++)
        {
            EXPECT_EQ(nullptr, configWithResources.getProxy(i)->getInputBuffer().data());
            EXPECT_EQ(nullptr, configWithResources.getProxy(i)->getOutputBuffer().data());
            EXPECT_NE(nullptr, configWithResources.getProxy(i)->getSocket());
        }
    }
}

TEST_F(UdpProxyConfigTest, setListener)
{
    ::someip::NetworkListenerMock listener;
    for (size_t i = 0U; i < UdpProxyConfigInstance::PROXIES_SIZE; ++i)
    {
        EXPECT_EQ(nullptr, _config.getProxy(i)->getInputBuffer().data());
        EXPECT_EQ(nullptr, _config.getProxy(i)->getOutputBuffer().data());
    }
    _config.setListener(listener);
    for (size_t i = 0U; i < UdpProxyConfigInstance::PROXIES_SIZE; ++i)
    {
        EXPECT_EQ(static_cast<INetworkListener*>(&listener), _config.getProxy(i)->getListener());
        EXPECT_EQ(&_config.inputBuffer[0], _config.getProxy(i)->getInputBuffer().data());
        EXPECT_EQ(&_config.outputBuffer[0], _config.getProxy(i)->getOutputBuffer().data());
    }
}

TEST_F(UdpProxyConfigTest, close)
{
    ::someip::NetworkListenerMock listener;
    _config.setListener(listener);
    _config.addToServers(_config.proxyArray[1U]);
    EXPECT_EQ(1U, _config.serverProxyList.size());
    EXPECT_EQ(&_config.proxyArray[1U], _config.serverProxyList[0U]);

    auto& socketMock0
        = dynamic_cast<::udp::AbstractDatagramSocketMock&>(*_config.proxyArray[0U].getSocket());
    auto& socketMock1
        = dynamic_cast<::udp::AbstractDatagramSocketMock&>(*_config.proxyArray[1U].getSocket());

    EXPECT_CALL(socketMock0, isBound()).WillRepeatedly(Return(false));
    EXPECT_CALL(socketMock1, isBound()).WillRepeatedly(Return(true));
    EXPECT_CALL(socketMock1, close()).Times(1U);
    _config.close();

    EXPECT_TRUE(_config.serverProxyList.empty());
}

TEST_F(UdpProxyConfigTest, shutdownPort)
{
    ::someip::NetworkListenerMock listener;
    _config.setListener(listener);

    _config.serverProxyList.push_back(&_config.proxyArray[1U]);
    _config.proxyArray[1].incRefCounter();

    uint16_t const localPort = 12U;

    auto& socketMock0
        = dynamic_cast<::udp::AbstractDatagramSocketMock&>(*_config.proxyArray[0U].getSocket());
    auto& socketMock1
        = dynamic_cast<::udp::AbstractDatagramSocketMock&>(*_config.proxyArray[1U].getSocket());

    EXPECT_CALL(socketMock0, isBound()).WillRepeatedly(Return(false));
    EXPECT_CALL(socketMock1, isBound()).WillRepeatedly(Return(true));
    EXPECT_CALL(socketMock1, getLocalPort()).WillOnce(Return(localPort));
    EXPECT_CALL(socketMock1, close()).Times(1U);
    _config.shutdownPort(localPort);

    EXPECT_TRUE(_config.serverProxyList.empty());

    // check if proxy refCount is 0
    EXPECT_CALL(socketMock1, isBound()).WillOnce(Return(true));
    EXPECT_CALL(socketMock1, close()).Times(1U);
    _config.proxyArray[1].tryClose();
}

TEST_F(UdpProxyConfigTest, addToServers)
{
    ::someip::NetworkListenerMock listener;
    _config.setListener(listener);

    _config.addToServers(_config.proxyArray[1U]);

    EXPECT_EQ(1U, _config.serverProxyList.size());
    EXPECT_EQ(&_config.proxyArray[1U], _config.serverProxyList[0U]);

    _config.proxyArray[1U].tryClose();
}

} // namespace
