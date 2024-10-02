// Copyright 2024 Accenture.

/**
 * Contains tests for QueuedTransportLayer.
 * \ingroup transport
 */

#include "transport/QueuedTransportLayer.h"

#include "transport/AbstractTransportLayerMock.h"
#include "transport/TransportConfiguration.h"
#include "transport/TransportMessageProcessedListenerMock.h"

#include <gmock/gmock.h>

#include <cstdint>

using namespace ::testing;
using namespace transport;

namespace
{
class QueuedTransportLayerTest : public ::testing::Test
{
public:
    using JobPool = ::estd::declare::object_pool<
        TransportMessageSendJob,
        TransportConfiguration::MAXIMUM_NUMBER_OF_TRANSPORT_MESSAGES>;

    static uint8_t const BUFFER_LENGTH = 16U; // Max. payload length == 8

    QueuedTransportLayerTest()
    {
        _jobPool.clear();
        QueuedTransportLayer::initializeJobPool(_jobPool);
    }

protected:
    AbstractTransportLayerMock _transportLayerMock{0};
    StrictMock<TransportMessageProcessedListenerMock> _processedListener;
    uint8_t buf[BUFFER_LENGTH];
    JobPool _jobPool;
    QueuedTransportLayer _queued{_transportLayerMock};
};

void shutdownDummy(AbstractTransportLayer&) {}

TEST_F(QueuedTransportLayerTest, does_noting_when_not_initialized)
{
    TransportMessage msg;
    QueuedTransportLayer::initializeJobPool();
    EXPECT_EQ(0U, _queued.numberOfAvailableSendJobs());
    EXPECT_EQ(AbstractTransportLayer::ErrorCode::TP_SEND_FAIL, _queued.send(msg, 0L));
}

TEST_F(QueuedTransportLayerTest, TestConstructor)
{
    ASSERT_EQ(0U, _queued.numberOfJobsSent());
    ASSERT_EQ(0U, _queued.numberOfJobsToBeSent());

    EXPECT_CALL(_transportLayerMock, init())
        .Times(1)
        .WillOnce(Return(AbstractTransportLayer::ErrorCode::TP_OK));
    ASSERT_EQ(AbstractTransportLayer::ErrorCode::TP_OK, _queued.init());

    EXPECT_CALL(_transportLayerMock, shutdown(_)).Times(1).WillOnce(Return(true));

    AbstractTransportLayer::ShutdownDelegate shut
        = AbstractTransportLayer::ShutdownDelegate::create<&shutdownDummy>();

    ASSERT_TRUE(_queued.shutdown(shut));
}

TEST_F(QueuedTransportLayerTest, TestSendOk)
{
    TransportMessage msg;
    msg.init(buf, BUFFER_LENGTH);
    EXPECT_CALL(_transportLayerMock, send(_, _))
        .Times(1)
        .WillOnce(Return(AbstractTransportLayer::ErrorCode::TP_OK));

    ASSERT_EQ(AbstractTransportLayer::ErrorCode::TP_OK, _queued.send(msg, &_processedListener));
    ASSERT_EQ(1U, _queued.numberOfJobsSent());
    ASSERT_EQ(0U, _queued.numberOfJobsToBeSent());
}

TEST_F(QueuedTransportLayerTest, TestSendQueueFull)
{
    TransportMessage msg;
    msg.init(buf, BUFFER_LENGTH);
    EXPECT_CALL(_transportLayerMock, send(_, _))
        .Times(1)
        .WillOnce(Return(AbstractTransportLayer::ErrorCode::TP_QUEUE_FULL));

    ASSERT_EQ(AbstractTransportLayer::ErrorCode::TP_OK, _queued.send(msg, &_processedListener));
    ASSERT_EQ(0U, _queued.numberOfJobsSent());
    ASSERT_EQ(1U, _queued.numberOfJobsToBeSent());
}

TEST_F(QueuedTransportLayerTest, TestSendQueueFullUntilNoMoreSendJobsLeft)
{
    TransportMessage msg;
    msg.init(buf, BUFFER_LENGTH);
    EXPECT_CALL(_transportLayerMock, send(_, _))
        .Times(1)
        .WillOnce(Return(AbstractTransportLayer::ErrorCode::TP_QUEUE_FULL));

    ASSERT_EQ(AbstractTransportLayer::ErrorCode::TP_OK, _queued.send(msg, &_processedListener));
    ASSERT_EQ(0U, _queued.numberOfJobsSent());
    ASSERT_EQ(1U, _queued.numberOfJobsToBeSent());
    size_t expectedJobsToBeSentSize = 1U;
    while (_queued.numberOfAvailableSendJobs() > 0)
    {
        ASSERT_EQ(AbstractTransportLayer::ErrorCode::TP_OK, _queued.send(msg, &_processedListener));
        ++expectedJobsToBeSentSize;
        ASSERT_EQ(0U, _queued.numberOfJobsSent());
        ASSERT_EQ(expectedJobsToBeSentSize, _queued.numberOfJobsToBeSent());
    }
    // next send will fail because no more jobs are available
    ASSERT_EQ(
        AbstractTransportLayer::ErrorCode::TP_SEND_FAIL, _queued.send(msg, &_processedListener));
    AbstractTransportLayerMock layer2((0));
    QueuedTransportLayer queued2(layer2);
    ASSERT_EQ(
        AbstractTransportLayer::ErrorCode::TP_SEND_FAIL, queued2.send(msg, &_processedListener));
}

TEST_F(QueuedTransportLayerTest, TestSendInvalidStatus)
{
    TransportMessage msg;
    msg.init(buf, BUFFER_LENGTH);
    EXPECT_CALL(_transportLayerMock, send(_, _))
        .Times(1)
        .WillOnce(Return(AbstractTransportLayer::ErrorCode::TP_SEND_FAIL));

    ASSERT_EQ(
        AbstractTransportLayer::ErrorCode::TP_SEND_FAIL, _queued.send(msg, &_processedListener));
    ASSERT_EQ(0U, _queued.numberOfJobsSent());
    ASSERT_EQ(0U, _queued.numberOfJobsToBeSent());
}

TEST_F(QueuedTransportLayerTest, TestSendPendingNotEmpty)
{
    TransportMessage msg;
    msg.init(buf, BUFFER_LENGTH);
    EXPECT_CALL(_transportLayerMock, send(_, _))
        .Times(1)
        .WillOnce(Return(AbstractTransportLayer::ErrorCode::TP_QUEUE_FULL));

    // put one message into the queue
    ASSERT_EQ(AbstractTransportLayer::ErrorCode::TP_OK, _queued.send(msg, &_processedListener));
    ASSERT_EQ(0U, _queued.numberOfJobsSent());
    ASSERT_EQ(1U, _queued.numberOfJobsToBeSent());

    uint8_t buf2[BUFFER_LENGTH];
    TransportMessage msg2;
    msg2.init(buf2, BUFFER_LENGTH);
    ASSERT_EQ(AbstractTransportLayer::ErrorCode::TP_OK, _queued.send(msg2, &_processedListener));
    ASSERT_EQ(0U, _queued.numberOfJobsSent());
    ASSERT_EQ(2U, _queued.numberOfJobsToBeSent());
}

TEST_F(QueuedTransportLayerTest, MessageProcessed)
{
    TransportMessage msg;
    msg.init(buf, BUFFER_LENGTH);
    EXPECT_CALL(_transportLayerMock, send(_, _))
        .Times(1)
        .WillOnce(Return(AbstractTransportLayer::ErrorCode::TP_OK));

    ASSERT_EQ(AbstractTransportLayer::ErrorCode::TP_OK, _queued.send(msg, &_processedListener));
    ASSERT_EQ(1U, _queued.numberOfJobsSent());
    ASSERT_EQ(0U, _queued.numberOfJobsToBeSent());

    EXPECT_CALL(
        _processedListener,
        transportMessageProcessed(
            _, Eq(ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR)))
        .Times(1);

    _queued.transportMessageProcessed(
        msg, ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR);
    ASSERT_EQ(0U, _queued.numberOfJobsSent());
    ASSERT_EQ(0U, _queued.numberOfJobsToBeSent());
}

TEST_F(QueuedTransportLayerTest, MessageProcessedNoListener)
{
    TransportMessage msg;
    msg.init(buf, BUFFER_LENGTH);
    EXPECT_CALL(_transportLayerMock, send(_, _))
        .Times(1)
        .WillOnce(Return(AbstractTransportLayer::ErrorCode::TP_OK));

    ASSERT_EQ(AbstractTransportLayer::ErrorCode::TP_OK, _queued.send(msg, 0));
    ASSERT_EQ(1U, _queued.numberOfJobsSent());
    ASSERT_EQ(0U, _queued.numberOfJobsToBeSent());

    EXPECT_CALL(
        _processedListener,
        transportMessageProcessed(
            _, Eq(ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR)))
        .Times(0);
    _queued.transportMessageProcessed(
        msg, ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR);
    ASSERT_EQ(0U, _queued.numberOfJobsSent());
    ASSERT_EQ(0U, _queued.numberOfJobsToBeSent());
}

TEST_F(QueuedTransportLayerTest, MessageProcessedNoListenerSendFail)
{
    TransportMessage msg;
    msg.init(buf, BUFFER_LENGTH);
    EXPECT_CALL(_transportLayerMock, send(_, _))
        .Times(1)
        .WillOnce(Return(AbstractTransportLayer::ErrorCode::TP_SEND_FAIL));

    ASSERT_EQ(AbstractTransportLayer::ErrorCode::TP_SEND_FAIL, _queued.send(msg, 0));
    ASSERT_EQ(0U, _queued.numberOfJobsSent());
    ASSERT_EQ(0U, _queued.numberOfJobsToBeSent());

    EXPECT_CALL(
        _processedListener,
        transportMessageProcessed(
            _, Eq(ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR)))
        .Times(0);
}

TEST_F(QueuedTransportLayerTest, MessageProcessedNoListenerSendFailWithPendingSendJob)
{
    TransportMessage msg;
    msg.init(buf, BUFFER_LENGTH);
    EXPECT_CALL(_transportLayerMock, send(_, _))
        .Times(1)
        .WillOnce(Return(AbstractTransportLayer::ErrorCode::TP_OK));
    ASSERT_EQ(AbstractTransportLayer::ErrorCode::TP_OK, _queued.send(msg, 0));
    EXPECT_CALL(_transportLayerMock, send(_, _))
        .Times(1)
        .WillOnce(Return(AbstractTransportLayer::ErrorCode::TP_QUEUE_FULL));
    ASSERT_EQ(AbstractTransportLayer::ErrorCode::TP_OK, _queued.send(msg, 0));
    ASSERT_EQ(1U, _queued.numberOfJobsSent());
    ASSERT_EQ(1U, _queued.numberOfJobsToBeSent());

    EXPECT_CALL(
        _processedListener,
        transportMessageProcessed(
            _, Eq(ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR)))
        .Times(0);
    EXPECT_CALL(_transportLayerMock, send(_, _))
        .Times(1)
        .WillOnce(Return(AbstractTransportLayer::ErrorCode::TP_SEND_FAIL));
    _queued.transportMessageProcessed(
        msg, ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR);
}

TEST_F(QueuedTransportLayerTest, MessageProcessedNotFoundInSentJobs)
{
    TransportMessage msg;
    msg.init(buf, BUFFER_LENGTH);
    EXPECT_CALL(_transportLayerMock, send(_, _))
        .Times(1)
        .WillOnce(Return(AbstractTransportLayer::ErrorCode::TP_OK));

    ASSERT_EQ(AbstractTransportLayer::ErrorCode::TP_OK, _queued.send(msg, &_processedListener));
    ASSERT_EQ(1U, _queued.numberOfJobsSent());
    ASSERT_EQ(0U, _queued.numberOfJobsToBeSent());

    EXPECT_CALL(
        _processedListener,
        transportMessageProcessed(
            _, Eq(ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR)))
        .Times(0);

    uint8_t buf2[BUFFER_LENGTH];
    TransportMessage msg2;
    msg2.init(buf2, BUFFER_LENGTH);

    _queued.transportMessageProcessed(
        msg2, ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR);
    ASSERT_EQ(1U, _queued.numberOfJobsSent());
    ASSERT_EQ(0U, _queued.numberOfJobsToBeSent());
}

TEST_F(QueuedTransportLayerTest, MessageProcessedJobsToBeSentNotEmptyFailedSecondTime)
{
    TransportMessage msg;
    msg.init(buf, BUFFER_LENGTH);
    EXPECT_CALL(_transportLayerMock, send(_, _))
        .Times(1)
        .WillOnce(Return(AbstractTransportLayer::ErrorCode::TP_QUEUE_FULL));

    ASSERT_EQ(AbstractTransportLayer::ErrorCode::TP_OK, _queued.send(msg, &_processedListener));
    ASSERT_EQ(0U, _queued.numberOfJobsSent());
    ASSERT_EQ(1U, _queued.numberOfJobsToBeSent());

    EXPECT_CALL(
        _processedListener,
        transportMessageProcessed(
            _, Eq(ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR)))
        .Times(0);

    uint8_t buf2[BUFFER_LENGTH];
    TransportMessage msg2;
    msg2.init(buf2, BUFFER_LENGTH);

    EXPECT_CALL(_transportLayerMock, send(_, _))
        .Times(1)
        .WillOnce(Return(AbstractTransportLayer::ErrorCode::TP_QUEUE_FULL));
    _queued.transportMessageProcessed(
        msg2, ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR);
    ASSERT_EQ(0U, _queued.numberOfJobsSent());
    ASSERT_EQ(1U, _queued.numberOfJobsToBeSent());
}

TEST_F(QueuedTransportLayerTest, MessageProcessedJobsToBeSentNotEmptySuccessSecondTime)
{
    TransportMessage msg;
    msg.init(buf, BUFFER_LENGTH);
    EXPECT_CALL(_transportLayerMock, send(_, _))
        .Times(1)
        .WillOnce(Return(AbstractTransportLayer::ErrorCode::TP_QUEUE_FULL));

    ASSERT_EQ(AbstractTransportLayer::ErrorCode::TP_OK, _queued.send(msg, &_processedListener));
    ASSERT_EQ(0U, _queued.numberOfJobsSent());
    ASSERT_EQ(1U, _queued.numberOfJobsToBeSent());

    EXPECT_CALL(
        _processedListener,
        transportMessageProcessed(
            _, Eq(ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR)))
        .Times(0);

    uint8_t buf2[BUFFER_LENGTH];
    TransportMessage msg2;
    msg2.init(buf2, BUFFER_LENGTH);

    EXPECT_CALL(_transportLayerMock, send(_, _))
        .Times(1)
        .WillOnce(Return(AbstractTransportLayer::ErrorCode::TP_OK));
    _queued.transportMessageProcessed(
        msg2, ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR);
    ASSERT_EQ(1U, _queued.numberOfJobsSent());
    ASSERT_EQ(0U, _queued.numberOfJobsToBeSent());
}

TEST_F(QueuedTransportLayerTest, MessageProcessedJobsToBeSentNotEmptyFailedSecondTimeFatal)
{
    TransportMessage msg;
    msg.init(buf, BUFFER_LENGTH);
    EXPECT_CALL(_transportLayerMock, send(_, _))
        .Times(1)
        .WillOnce(Return(AbstractTransportLayer::ErrorCode::TP_QUEUE_FULL));

    ASSERT_EQ(AbstractTransportLayer::ErrorCode::TP_OK, _queued.send(msg, &_processedListener));
    ASSERT_EQ(0U, _queued.numberOfJobsSent());
    ASSERT_EQ(1U, _queued.numberOfJobsToBeSent());

    EXPECT_CALL(
        _processedListener,
        transportMessageProcessed(
            _, Eq(ITransportMessageProcessedListener::ProcessingResult::PROCESSED_ERROR)))
        .Times(1);

    uint8_t buf2[BUFFER_LENGTH];
    TransportMessage msg2;
    msg2.init(buf2, BUFFER_LENGTH);

    EXPECT_CALL(_transportLayerMock, send(_, _))
        .Times(1)
        .WillOnce(Return(AbstractTransportLayer::ErrorCode::TP_SEND_FAIL));
    _queued.transportMessageProcessed(
        msg2, ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR);
    ASSERT_EQ(0U, _queued.numberOfJobsSent());
    ASSERT_EQ(0U, _queued.numberOfJobsToBeSent());
}

} // anonymous namespace
