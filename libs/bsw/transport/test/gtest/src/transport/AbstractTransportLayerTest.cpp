// Copyright 2024 Accenture.

/**
 * Contains
 * \file
 * \ingroup
 */

#include "transport/AbstractTransportLayerMock.h"
#include "transport/TransportMessage.h"
#include "transport/TransportMessageListenerMock.h"
#include "transport/TransportMessageProviderMock.h"

#include <gmock/gmock.h>
using namespace ::transport;
using namespace ::testing;

namespace
{
class TestTransportLayer : public AbstractTransportLayer
{
public:
    using AbstractTransportLayer::getProvidingListenerHelper;
    using AbstractTransportLayer::shutdownCompleteDummy;

    TestTransportLayer() : AbstractTransportLayer(0) {}

    virtual ErrorCode send(
        TransportMessage& transportMessage,
        ITransportMessageProcessedListener* pNotificationListener)
    {
        return AbstractTransportLayer::ErrorCode::TP_OK;
    }
};

class AbstractTransportLayerTest : public ::testing::Test
{
public:
    AbstractTransportLayerTest();
    ~AbstractTransportLayerTest();

protected:
    AbstractTransportLayerMock* impl;
    StrictMock<TransportMessageProviderMock> provider;
    StrictMock<TransportMessageListenerMock> listener;
};

AbstractTransportLayerTest::AbstractTransportLayerTest()
{
    impl = new AbstractTransportLayerMock(0);
    impl->setTransportMessageListener(&listener);
    impl->setTransportMessageProvider(&provider);
}

AbstractTransportLayerTest::~AbstractTransportLayerTest() { delete impl; }

TEST_F(AbstractTransportLayerTest, TestHelperMethods)
{
    EXPECT_CALL(provider, getTransportMessage(_, Eq(1U), Eq(2U), Eq(3U), _, _))
        .Times(1)
        .WillOnce(Return(ITransportMessageProvidingListener::ErrorCode::TPMSG_OK));

    ITransportMessageProvidingListener& listenerHelper = impl->getProvidingListenerHelper_impl();

    TransportMessage* msg;
    ASSERT_EQ(
        ITransportMessageProvidingListener::ErrorCode::TPMSG_OK,
        listenerHelper.getTransportMessage(0, 1, 2, 3, {}, msg));

    TransportMessage tmp;

    EXPECT_CALL(provider, releaseTransportMessage(_)).Times(1);
    listenerHelper.releaseTransportMessage(tmp);

    EXPECT_CALL(listener, messageReceived(_, _, _))
        .Times(1)
        .WillOnce(Return(ITransportMessageListener::ReceiveResult::RECEIVED_NO_ERROR));

    ASSERT_EQ(
        ITransportMessageListener::ReceiveResult::RECEIVED_NO_ERROR,
        listenerHelper.messageReceived(0, tmp, 0));
}

TEST_F(AbstractTransportLayerTest, TestHelperMethodsNoProvider)
{
    impl->setTransportMessageProvider(0L);
    impl->setTransportMessageListener(0L);

    ITransportMessageProvidingListener& listenerHelper = impl->getProvidingListenerHelper_impl();

    TransportMessage* msg = 0;
    ASSERT_EQ(
        ITransportMessageProvidingListener::ErrorCode::TPMSG_NO_MSG_AVAILABLE,
        listenerHelper.getTransportMessage(0, 1, 2, 3, {}, msg));
    ASSERT_EQ(0L, msg);

    TransportMessage tmp;

    EXPECT_CALL(provider, releaseTransportMessage(_)).Times(0);
    listenerHelper.releaseTransportMessage(tmp);

    ASSERT_EQ(
        ITransportMessageListener::ReceiveResult::RECEIVED_ERROR,
        listenerHelper.messageReceived(0, tmp, 0));
}

/**
 * \desc
 * This test will make sure that transport::AbstractTransportLayer::shutdownCompleteDummy is called
 * once.
 */
TEST_F(AbstractTransportLayerTest, TestShutdownCompleteDummy)
{
    AbstractTransportLayer& l = *impl;
    EXPECT_CALL(*impl, shutdown(_))
        .Times(1)
        .WillOnce(DoAll(InvokeArgument<0>(ByRef(l)), Return(false)));
    impl->shutdown(transport::AbstractTransportLayer::ShutdownDelegate::create<
                   &transport::AbstractTransportLayerMock::shutdownCompleteDummy>());
}

TEST_F(AbstractTransportLayerTest, TestDumpWithInitializedTransportMessageProvider)
{
    EXPECT_CALL(provider, dump()).Times(1);
    ASSERT_NO_THROW(impl->getProvidingListenerHelper_impl().dump());
}

TEST_F(AbstractTransportLayerTest, TestInitDefaultImplementation)
{
    TestTransportLayer tpLayer;
    ASSERT_EQ(AbstractTransportLayer::ErrorCode::TP_OK, tpLayer.init());
}

TEST_F(AbstractTransportLayerTest, TestShutdownDefaultImplementation)
{
    TestTransportLayer tpLayer;
    ASSERT_EQ(
        true,
        tpLayer.shutdown(TestTransportLayer::ShutdownDelegate::create<
                         &TestTransportLayer::shutdownCompleteDummy>()));
}

TEST_F(AbstractTransportLayerTest, TestDumpWithUninitializedTransportMessageProvider)
{
    TestTransportLayer tpLayer;
    ASSERT_NO_THROW(tpLayer.getProvidingListenerHelper().dump());
}

} // anonymous namespace
