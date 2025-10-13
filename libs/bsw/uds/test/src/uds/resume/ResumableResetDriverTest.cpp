// Copyright 2024 Accenture.

#include "uds/resume/ResumableResetDriver.h"

#include "uds/resume/DiagDispatcherMock.h"
#include "uds/resume/IResumableResetDriverPersistence.h"
#include "uds/session/DiagSessionManagerMock.h"

#include <async/AsyncMock.h>
#include <async/TestContext.h>

#include <gtest/esr_extensions.h>
#include <gtest/gtest.h>

namespace
{
using namespace ::transport;
using namespace ::uds;
using namespace ::testing;

class TestDiagDispatcher : public DiagDispatcherMock
{
public:
    TestDiagDispatcher() : DiagDispatcherMock(fSessionManagerMock) {}

private:
    DiagSessionManagerMock fSessionManagerMock;
    DiagJobRoot fJobRoot;
};

class TestLifecycleAdmin : public IUdsLifecycleConnector
{
public:
    TestLifecycleAdmin()
    : fModeChangePossible(true)
    , fShutdownResult(true)
    , fShutdownType(NO_SHUTDOWN)
    , fTimeout(0xffffffffU)
    {}

    void setModeChangePossible(bool modeChangePossible)
    {
        fModeChangePossible = modeChangePossible;
    }

    void setShutdownResult(bool shutdownResult) { fShutdownResult = shutdownResult; }

    ShutdownType getShutdownType() const { return fShutdownType; }

    uint32_t getTimeout() const { return fTimeout; }

    virtual bool requestShutdown(ShutdownType type, uint32_t timeout)
    {
        fShutdownType = type;
        fTimeout      = timeout;
        return fShutdownResult;
    }

    /* 10 is some arbitrary number, just override */
    virtual bool requestPowerdown(bool /* rapid */, uint8_t& /* time */) { return true; }

    virtual bool isModeChangePossible() const { return fModeChangePossible; }

    bool fModeChangePossible;
    bool fShutdownResult;
    ShutdownType fShutdownType;
    uint32_t fTimeout;
};

class TestResumableResetDriverPersistence : public IResumableResetDriverPersistence
{
public:
    ::transport::TransportMessage& getTransportMessage() { return fTransportMessage; }

    bool readRequest(::transport::TransportMessage& message) const override
    {
        if (fTransportMessage.getSourceId() != ::transport::TransportMessage::INVALID_ADDRESS)
        {
            message.setSourceAddress(fTransportMessage.getSourceId());
            message.setTargetAddress(fTransportMessage.getTargetId());
            message.append(fTransportMessage.getPayload(), fTransportMessage.getPayloadLength());
            message.setPayloadLength(fTransportMessage.getPayloadLength());
            return true;
        }
        else
        {
            return false;
        }
    }

    void writeRequest(::transport::TransportMessage const& message) override
    {
        fTransportMessage.setSourceAddress(message.getSourceId());
        fTransportMessage.setTargetAddress(message.getTargetId());
        fTransportMessage.append(message.getPayload(), message.getPayloadLength());
        fTransportMessage.setPayloadLength(message.getPayloadLength());
    }

    void clear() override
    {
        fTransportMessage.init(
            fTransportMessage.getPayload(), fTransportMessage.getPayloadLength());
        fTransportMessage.setSourceAddress(::transport::TransportMessage::INVALID_ADDRESS);
    }

private:
    BufferedTransportMessage<8> fTransportMessage;
};

struct ResumableResetDriverTest : ::testing::Test
{
    TestDiagDispatcher fDiagDispatcher;
    TestLifecycleAdmin fLifecycleAdmin;
    TestResumableResetDriverPersistence fResumableResetDriverPersistence;
    BufferedTransportMessage<6> fTransportMessage;
    ::async::ContextType _context;
    ::async::AsyncMock _asyncMock;
    ::async::TestContext _testContext;

    ResumableResetDriverTest() : _context(), _testContext(_context) {}
};

TEST_F(ResumableResetDriverTest, driverStoresTransportMessageAndForcesShutdown)
{
    ResumableResetDriver cut(
        fLifecycleAdmin, _context, fResumableResetDriverPersistence, fTransportMessage);
    cut.resume(fDiagDispatcher);
    EXPECT_CALL(fDiagDispatcher, resume(_, _)).Times(0);
    fTransportMessage.setSourceAddress(0x1234);
    fTransportMessage.setTargetAddress(0x5678);
    uint8_t const payload[] = {0x22, 0x34};
    fTransportMessage.append(payload, sizeof(payload));
    fTransportMessage.setPayloadLength(sizeof(payload));
    ASSERT_TRUE(cut.prepareReset(&fTransportMessage));
    EXPECT_EQ(0x1234, fResumableResetDriverPersistence.getTransportMessage().getSourceId());
    EXPECT_EQ(0x5678, fResumableResetDriverPersistence.getTransportMessage().getTargetId());
    EXPECT_EQ(
        sizeof(payload), fResumableResetDriverPersistence.getTransportMessage().getPayloadLength());
    EXPECT_EQ(
        0,
        ::memcmp(
            payload,
            fResumableResetDriverPersistence.getTransportMessage().getPayload(),
            sizeof(payload)));
    cut.reset();
    EXPECT_EQ(IUdsLifecycleConnector::HARD_RESET, fLifecycleAdmin.getShutdownType());
    EXPECT_EQ(100U, fLifecycleAdmin.getTimeout());
}

TEST_F(ResumableResetDriverTest, driverResetsStoredTransportMessageIfNoneGivenInPrepareReset)
{
    ResumableResetDriver cut(
        fLifecycleAdmin, _context, fResumableResetDriverPersistence, fTransportMessage);
    cut.resume(fDiagDispatcher);
    EXPECT_CALL(fDiagDispatcher, resume(_, _)).Times(0);
    fResumableResetDriverPersistence.getTransportMessage().setSourceAddress(1U);
    ASSERT_TRUE(cut.prepareReset());
    EXPECT_EQ(
        uint32_t(TransportMessage::INVALID_ADDRESS),
        fResumableResetDriverPersistence.getTransportMessage().getSourceId());
    cut.reset();
    EXPECT_EQ(IUdsLifecycleConnector::HARD_RESET, fLifecycleAdmin.getShutdownType());
    EXPECT_EQ(100U, fLifecycleAdmin.getTimeout());
}

TEST_F(ResumableResetDriverTest, driverResumesTransportMessageOnStartupIfStored)
{
    ResumableResetDriver cut(
        fLifecycleAdmin, _context, fResumableResetDriverPersistence, fTransportMessage);
    fResumableResetDriverPersistence.getTransportMessage().setSourceAddress(0x1425);
    fResumableResetDriverPersistence.getTransportMessage().setTargetAddress(0x6823);
    uint8_t const payload[] = {0x34, 0x38};
    fResumableResetDriverPersistence.getTransportMessage().append(payload, sizeof(payload));
    fResumableResetDriverPersistence.getTransportMessage().setPayloadLength(sizeof(payload));
    EXPECT_CALL(fDiagDispatcher, resume(Ref(fTransportMessage), _))
        .WillOnce(Return(AbstractTransportLayer::ErrorCode::TP_OK));
    cut.resume(fDiagDispatcher);
    EXPECT_EQ(
        uint32_t(TransportMessage::INVALID_ADDRESS),
        fResumableResetDriverPersistence.getTransportMessage().getSourceId());
}

TEST_F(ResumableResetDriverTest, driverClearsPersistenceOnStartupIfEmptyMessageStored)
{
    ResumableResetDriver cut(
        fLifecycleAdmin, _context, fResumableResetDriverPersistence, fTransportMessage);
    fResumableResetDriverPersistence.getTransportMessage().setSourceAddress(0x1425);
    fResumableResetDriverPersistence.getTransportMessage().setTargetAddress(0x6823);
    cut.resume(fDiagDispatcher);
    EXPECT_EQ(
        uint32_t(TransportMessage::INVALID_ADDRESS),
        fResumableResetDriverPersistence.getTransportMessage().getSourceId());
}

TEST_F(ResumableResetDriverTest, prepareResetFailsIfModeChangeIsNotAllowed)
{
    ResumableResetDriver cut(
        fLifecycleAdmin, _context, fResumableResetDriverPersistence, fTransportMessage);
    cut.resume(fDiagDispatcher);
    fTransportMessage.setSourceAddress(0x1234);
    fTransportMessage.setTargetAddress(0x5678);
    uint8_t const payload[] = {0x22, 0x34};
    fTransportMessage.append(payload, sizeof(payload));
    fTransportMessage.setPayloadLength(sizeof(payload));
    fLifecycleAdmin.setModeChangePossible(false);
    ASSERT_FALSE(cut.prepareReset(&fTransportMessage));
    EXPECT_EQ(
        uint32_t(TransportMessage::INVALID_ADDRESS),
        fResumableResetDriverPersistence.getTransportMessage().getSourceId());
}

TEST_F(ResumableResetDriverTest, driverAbortsResetIfShutdownFails)
{
    ResumableResetDriver cut(
        fLifecycleAdmin, _context, fResumableResetDriverPersistence, fTransportMessage);
    cut.resume(fDiagDispatcher);
    fTransportMessage.setSourceAddress(0x1234);
    fTransportMessage.setTargetAddress(0x5678);
    uint8_t const payload[] = {0x22, 0x34};
    fTransportMessage.append(payload, sizeof(payload));
    fTransportMessage.setPayloadLength(sizeof(payload));
    cut.prepareReset(&fTransportMessage);
    EXPECT_EQ(0x1234, fResumableResetDriverPersistence.getTransportMessage().getSourceId());
    EXPECT_EQ(0x5678, fResumableResetDriverPersistence.getTransportMessage().getTargetId());
    EXPECT_EQ(
        sizeof(payload), fResumableResetDriverPersistence.getTransportMessage().getPayloadLength());
    EXPECT_EQ(
        0,
        ::memcmp(
            payload,
            fResumableResetDriverPersistence.getTransportMessage().getPayload(),
            sizeof(payload)));
    fLifecycleAdmin.setShutdownResult(false);
    cut.reset();
    EXPECT_EQ(
        uint32_t(TransportMessage::INVALID_ADDRESS),
        fResumableResetDriverPersistence.getTransportMessage().getSourceId());
}

TEST_F(ResumableResetDriverTest, driverListensToLifecycleEvents)
{
    ResumableResetDriver cut(
        fLifecycleAdmin,
        _context,
        fResumableResetDriverPersistence,
        fTransportMessage,
        200U,
        IUdsLifecycleConnector::POWER_DOWN,
        0x6677,
        300U);
    cut.init(fDiagDispatcher);
    fResumableResetDriverPersistence.getTransportMessage().setSourceAddress(0x6321);
    fResumableResetDriverPersistence.getTransportMessage().setTargetAddress(0x6823);
    uint8_t const payload[] = {0x34, 0x38};
    fResumableResetDriverPersistence.getTransportMessage().append(payload, sizeof(payload));
    fResumableResetDriverPersistence.getTransportMessage().setPayloadLength(sizeof(payload));
    EXPECT_CALL(fDiagDispatcher, resume(_, _))
        .WillOnce(Return(AbstractTransportLayer::ErrorCode::TP_OK));
    cut.lifecycleComplete();
    EXPECT_EQ(
        uint32_t(TransportMessage::INVALID_ADDRESS),
        fResumableResetDriverPersistence.getTransportMessage().getSourceId());
    cut.shutdown();
}

TEST_F(
    ResumableResetDriverTest,
    driverResumesTransportMessageImmediatelyAfterTimeoutInCaseOfNonEthObdSource)
{
    ResumableResetDriver cut(
        fLifecycleAdmin,
        _context,
        fResumableResetDriverPersistence,
        fTransportMessage,
        200U,
        IUdsLifecycleConnector::POWER_DOWN,
        0x6677,
        300U);
    fResumableResetDriverPersistence.getTransportMessage().setSourceAddress(0x1425);
    fResumableResetDriverPersistence.getTransportMessage().setTargetAddress(0x6823);
    uint8_t const payload[] = {0x34, 0x38};
    fResumableResetDriverPersistence.getTransportMessage().append(payload, sizeof(payload));
    fResumableResetDriverPersistence.getTransportMessage().setPayloadLength(sizeof(payload));
    EXPECT_CALL(fDiagDispatcher, resume(Ref(fTransportMessage), _))
        .WillOnce(Return(AbstractTransportLayer::ErrorCode::TP_OK));
    cut.resume(fDiagDispatcher);
    EXPECT_EQ(
        uint32_t(TransportMessage::INVALID_ADDRESS),
        fResumableResetDriverPersistence.getTransportMessage().getSourceId());
}

TEST_F(ResumableResetDriverTest, driverResumesTransportMessageAfterTimeoutInCaseOfEthObdSource)
{
    ResumableResetDriver cut(
        fLifecycleAdmin,
        _context,
        fResumableResetDriverPersistence,
        fTransportMessage,
        200U,
        IUdsLifecycleConnector::POWER_DOWN,
        0x6677,
        300U);
    fResumableResetDriverPersistence.getTransportMessage().setSourceAddress(0x6677);
    fResumableResetDriverPersistence.getTransportMessage().setTargetAddress(0x6823);
    uint8_t const payload[] = {0x34, 0x38};
    fResumableResetDriverPersistence.getTransportMessage().append(payload, sizeof(payload));
    fResumableResetDriverPersistence.getTransportMessage().setPayloadLength(sizeof(payload));

    ::async::RunnableType* runnable;
    EXPECT_CALL(_asyncMock, schedule(_context, _, _, 300, ::async::TimeUnit::MILLISECONDS))
        .WillOnce(WithArg<1>(SaveRef<0>(&runnable)));
    cut.resume(fDiagDispatcher);
    EXPECT_EQ(
        uint32_t(TransportMessage::INVALID_ADDRESS),
        fResumableResetDriverPersistence.getTransportMessage().getSourceId());
    EXPECT_CALL(fDiagDispatcher, resume(Ref(fTransportMessage), _))
        .WillOnce(Return(AbstractTransportLayer::ErrorCode::TP_OK));
    runnable->execute();
}

TEST_F(
    ResumableResetDriverTest,
    driverResumesTransportMessageWithoutTimeoutInCaseOfEthObdSourceWithoutTimeout)
{
    declare::ResumableResetDriver<6> cut(
        fLifecycleAdmin,
        _context,
        fResumableResetDriverPersistence,
        200U,
        IUdsLifecycleConnector::POWER_DOWN,
        0x6677,
        0U);
    fResumableResetDriverPersistence.getTransportMessage().setSourceAddress(0x6677);
    fResumableResetDriverPersistence.getTransportMessage().setTargetAddress(0x6823);
    uint8_t const payload[] = {0x34, 0x38};
    fResumableResetDriverPersistence.getTransportMessage().append(payload, sizeof(payload));
    fResumableResetDriverPersistence.getTransportMessage().setPayloadLength(sizeof(payload));
    EXPECT_CALL(fDiagDispatcher, resume(_, _))
        .WillOnce(Return(AbstractTransportLayer::ErrorCode::TP_OK));
    cut.resume(fDiagDispatcher);
}

} // anonymous namespace
