// Copyright 2024 Accenture.

#include "io/DynamicClientCfg.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <iostream>

class DynamicClientTest : public testing::Test
{
public:
    DynamicClientTest() {}

    class IDynamicTestClient
    {
    public:
        virtual bool runAction()      = 0;
        virtual uint8_t getClientId() = 0;
    };

    class DynamicClientExample : public IDynamicTestClient
    {
    public:
        DynamicClientExample(uint8_t const clientId) : _clientId(clientId) {}

        bool runAction() final { return true; }

        uint8_t getClientId() final { return _clientId; }

    private:
        uint8_t const _clientId;
    };

private:
protected:
    dynamicClient<uint16_t, IDynamicTestClient, 3, 3> dut;
};

/**
 * Helper method to ease debugging of dynamicClient internals
 */
template<typename T, typename IDynamicClient, int Size, int dynamicChannels, int dynamicClientCount>
std::ostream& operator<<(
    std::ostream& os,
    dynamicClient<T, IDynamicClient, Size, dynamicChannels, dynamicClientCount> const& client)
{
    using ClientType = dynamicClient<T, IDynamicClient, Size, dynamicChannels, dynamicClientCount>;
    os << "*********************************************************************" << std::endl;
    os << "* Dynamic Client: size=" << Size << ", channels count=" << dynamicChannels
       << ", clients count=" << dynamicClientCount << std::endl;
    os << "* dynamicClientCountCfg=" << client.dynamicClientCountCfg << std::endl;

    // get count of registered dynamic clients:
    int registeredDynamicClientsCount = 0;
    for (int i = 0; i < client.dynamicClientCountCfg; ++i)
    {
        if (client.fpDynamicClient[i] != 0)
        {
            registeredDynamicClientsCount++;
        }
    }

    int registeredDynamicChannelCount = 0;
    for (int i = 0; i < dynamicChannels; ++i)
    {
        if (client.fDynamicChannelCfg[i].numberInsideClient != ClientType::dynamicClientInvalid)
        {
            registeredDynamicChannelCount++;
        }
    }
    os << "* registered clients count=" << registeredDynamicClientsCount
       << ", registered channels count=" << registeredDynamicChannelCount << std::endl;
    os << "*********************************************************************" << std::endl;
    return os;
}

TEST_F(DynamicClientTest, ConstructorTest)
{
    DynamicClientTest::DynamicClientExample client(1);
    // cout << dut << endl;
}

TEST_F(DynamicClientTest, UseCaseWith2Clients)
{
    DynamicClientTest::DynamicClientExample client1(1);
    DynamicClientTest::DynamicClientExample client2(2);

    EXPECT_TRUE(dut.setDynamicClient(0, 0, &client1));
    EXPECT_TRUE(dut.setDynamicClient(1, 1, &client1));
    EXPECT_TRUE(dut.setDynamicClient(2, 0, &client2));

    EXPECT_EQ(dut.getClientInstance(0)->getClientId(), 1);
    EXPECT_EQ(dut.getClientInstance(1)->getClientId(), 1);
    EXPECT_EQ(dut.getClientInstance(2)->getClientId(), 2);

    EXPECT_EQ(dut.getChannelInsideClient(0), 0);
    EXPECT_EQ(dut.getChannelInsideClient(1), 1);
    EXPECT_EQ(dut.getChannelInsideClient(2), 0);
}
