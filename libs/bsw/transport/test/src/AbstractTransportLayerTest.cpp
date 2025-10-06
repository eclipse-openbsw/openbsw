// Copyright 2024 Accenture.

/**
 * Contains
 * \file
 * \ingroup
 */

#include "gmock/gmock.h"
#include "transport/AbstractTransportLayerMock.h"
#include "transport/TransportMessage.h"
#include "transport/TransportMessageListenerMock.h"
#include "transport/TransportMessageProviderMock.h"

#include <memory>

using namespace ::transport;
using namespace ::testing;

namespace
{
class TestTransportLayer : public AbstractTransportLayer
{
public:
    using AbstractTransportLayer::shutdownCompleteDummy;

    TestTransportLayer() : AbstractTransportLayer(0) {}

    ErrorCode send(
        TransportMessage& /*transportMessage*/,
        ITransportMessageProcessedListener* /*pNotificationListener*/) override
    {
        return AbstractTransportLayer::ErrorCode::TP_OK;
    }
};

class AbstractTransportLayerTest : public ::testing::Test
{
public:
    AbstractTransportLayerTest();
    ~AbstractTransportLayerTest() override = default;

protected:
    std::unique_ptr<AbstractTransportLayerMock> impl;
    StrictMock<TransportMessageProviderMock> provider;
    StrictMock<TransportMessageListenerMock> listener;
};

AbstractTransportLayerTest::AbstractTransportLayerTest()
{
    impl = std::make_unique<AbstractTransportLayerMock>(0);
    impl->fProvidingListenerHelper.fpMessageListener = &listener;
    impl->fProvidingListenerHelper.fpMessageProvider = &provider;
}

/**
 * This test verifies whether
 * TransportMessageProvidingListenerHelper::getTransportMessage() triggers
 * ITransportMessageProvidingListener::getTransportMessage()
 * (ITransportMessageProvider::getTransportMessage()) and if
 * TransportMessageProvidingListenerHelper::messageReceived() triggers
 * ITransportMessageProvidingListener::releaseTransportMessage()
 * (ITransportMessageProvider::releaseTransportMessage()).
 */
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
        listenerHelper.messageReceived(0, tmp, nullptr));
}

/**
 * This test checks that when ITransportMessageProvider and
 * ITransportMessageListener are unset in AbstractTransportLayer it's underlying
 * TransportMessageProvidingListenerHelper::getTransportMessage() will return
 * NO_MSG_AVAILABLE and when calling
 * TransportMessageProvidingListenerHelper::releaseTransportMessage(), the
 * ITransportMessageProvider::releaseTransportMessage() is not called as it's
 * set to nullptr. Also in this scenario
 * TransportMessageProvidingListenerHelper::messageReceived() should return
 * RECEIVED_ERROR.
 */
TEST_F(AbstractTransportLayerTest, TestHelperMethodsNoProvider)
{
    impl->fProvidingListenerHelper.fpMessageProvider = nullptr;
    impl->fProvidingListenerHelper.fpMessageListener = nullptr;

    ITransportMessageProvidingListener& listenerHelper = impl->getProvidingListenerHelper_impl();

    TransportMessage* msg = nullptr;
    ASSERT_EQ(
        ITransportMessageProvidingListener::ErrorCode::TPMSG_NO_MSG_AVAILABLE,
        listenerHelper.getTransportMessage(0, 1, 2, 3, {}, msg));
    ASSERT_EQ(nullptr, msg);

    TransportMessage tmp;

    EXPECT_CALL(provider, releaseTransportMessage(_)).Times(0);
    listenerHelper.releaseTransportMessage(tmp);

    ASSERT_EQ(
        ITransportMessageListener::ReceiveResult::RECEIVED_ERROR,
        listenerHelper.messageReceived(0, tmp, nullptr));
}

/**
 * This test will make sure that
 * transport::AbstractTransportLayer::shutdownCompleteDummy is called once.
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

/**
 * Test if TransportMessageProvidingListenerHelper::dump() will trigger
 * underlying ITransportMessageProvider::dump()
 */
TEST_F(AbstractTransportLayerTest, TestDumpWithInitializedTransportMessageProvider)
{
    EXPECT_CALL(provider, dump()).Times(1);
    ASSERT_NO_THROW(impl->getProvidingListenerHelper_impl().dump());
}

/**
 * Default implementation for AbstractTransportLayer::init() should return
 * TP_OK.
 */
TEST_F(AbstractTransportLayerTest, TestInitDefaultImplementation)
{
    TestTransportLayer tpLayer;
    ASSERT_EQ(AbstractTransportLayer::ErrorCode::TP_OK, tpLayer.init());
}

/**
 * Default implementation for AbstractTransportLayer::shutdown() should return
 * true (SYNC_SHUTDOWN_COMPLETE).
 */
TEST_F(AbstractTransportLayerTest, TestShutdownDefaultImplementation)
{
    TestTransportLayer tpLayer;
    ASSERT_EQ(
        true,
        tpLayer.shutdown(TestTransportLayer::ShutdownDelegate::create<
                         &TestTransportLayer::shutdownCompleteDummy>()));
}

/**
 * Default implementation for TransportMessageProvidingListenerHelper::dump()
 * shouldn't throw.
 */
TEST_F(AbstractTransportLayerTest, TestDumpWithUninitializedTransportMessageProvider)
{
    TestTransportLayer tpLayer;
    ASSERT_NO_THROW(tpLayer.fProvidingListenerHelper.dump());
}

} // anonymous namespace
