// Copyright 2024 Accenture.

#include "lifecycle/LifecycleManager.h"

#include "lifecycle/LifecycleComponentMock.h"
#include "lifecycle/LifecycleListenerMock.h"

#include <async/AsyncMock.h>
#include <async/TestContext.h>
#include <util/estd/assert.h>

#include <gtest/esr_extensions.h>
#include <gtest/gtest.h>

namespace
{
using namespace ::lifecycle;
using namespace ::testing;

struct LifecycleManagerTest : public Test
{
    LifecycleManagerTest() { _context.handleAll(); }

    MOCK_METHOD0(getTimestamp, uint32_t());

    StrictMock<::async::AsyncMock> _asyncMock;
    StrictMock<LifecycleComponentMock> _componentMock1;
    StrictMock<LifecycleComponentMock> _componentMock2;
    StrictMock<LifecycleComponentMock> _componentMock3;
    LifecycleManager::GetTimestampType _getTimestamp{
        LifecycleManager::GetTimestampType::
            create<LifecycleManagerTest, &LifecycleManagerTest::getTimestamp>(*this)};
    ::async::TestContext _context{1U};
    StrictMock<LifecycleListenerMock> _listenerMock;
};

TEST_F(LifecycleManagerTest, testCompleteLifecycle)
{
    ::lifecycle::declare::LifecycleManager<4, 3, 2> cut(_context, _getTimestamp);

    cut.addLifecycleListener(_listenerMock);

    // No component info if not yet added
    EXPECT_EQ(0U, cut.getComponentCount());
    // No level yet
    EXPECT_EQ(0U, cut.getLevelCount());

    ILifecycleComponentCallback* callback1 = nullptr;
    EXPECT_CALL(_componentMock1, initCallback(_)).WillOnce(WithArg<0>(SaveRef<0>(&callback1)));
    cut.addComponent("comp1", _componentMock1, 1U);
    EXPECT_EQ(1U, cut.getComponentCount());
    EXPECT_EQ(1U, cut.getLevelCount());
    LifecycleManager::ComponentInfo const& componentInfo1 = cut.getComponentInfo(0U);
    EXPECT_EQ(0U, componentInfo1._transitionTimes[0U]);
    EXPECT_EQ(0U, componentInfo1._transitionTimes[1U]);
    EXPECT_EQ(0U, componentInfo1._transitionTimes[2U]);
    EXPECT_EQ(ILifecycleComponent::Transition::Type::INIT, componentInfo1._lastTransition);
    EXPECT_FALSE(componentInfo1._isTransitionPending);

    ILifecycleComponentCallback* callback2 = nullptr;
    EXPECT_CALL(_componentMock2, initCallback(_)).WillOnce(WithArg<0>(SaveRef<0>(&callback2)));
    cut.addComponent("comp2", _componentMock2, 1U);
    EXPECT_EQ(2U, cut.getComponentCount());
    EXPECT_EQ(1U, cut.getLevelCount());
    LifecycleManager::ComponentInfo const& componentInfo2 = cut.getComponentInfo(1U);

    ILifecycleComponentCallback* callback3 = nullptr;
    EXPECT_CALL(_componentMock3, initCallback(_)).WillOnce(WithArg<0>(SaveRef<0>(&callback3)));
    cut.addComponent("comp3", _componentMock3, 3U);
    EXPECT_EQ(3U, cut.getComponentCount());
    EXPECT_EQ(3U, cut.getLevelCount());
    LifecycleManager::ComponentInfo const& componentInfo3 = cut.getComponentInfo(2U);

    // Nothing will happen if transitionDone is called accidentally
    callback3->transitionDone(_componentMock3);

    // Start transition to level 3
    Sequence seq;
    cut.transitionToLevel(4U);

    // expect level 1 to be initialized
    EXPECT_CALL(*this, getTimestamp()).InSequence(seq).WillOnce(Return(100U));
    EXPECT_CALL(_componentMock1, getTransitionContext(ILifecycleComponent::Transition::Type::INIT))
        .WillOnce(Return(::async::CONTEXT_INVALID));
    EXPECT_CALL(_componentMock1, startTransition(ILifecycleComponent::Transition::Type::INIT));
    EXPECT_CALL(_componentMock2, getTransitionContext(ILifecycleComponent::Transition::Type::INIT))
        .WillOnce(Return(::async::CONTEXT_INVALID));
    EXPECT_CALL(_componentMock2, startTransition(ILifecycleComponent::Transition::Type::INIT));
    _context.execute();
    EXPECT_TRUE(componentInfo1._isTransitionPending);
    EXPECT_EQ(ILifecycleComponent::Transition::Type::INIT, componentInfo1._lastTransition);
    EXPECT_TRUE(componentInfo2._isTransitionPending);
    EXPECT_EQ(ILifecycleComponent::Transition::Type::INIT, componentInfo2._lastTransition);

    // expect level 1 to be run after initialization has been done
    EXPECT_CALL(*this, getTimestamp()).InSequence(seq).WillOnce(Return(200U));
    EXPECT_CALL(*this, getTimestamp()).InSequence(seq).WillOnce(Return(300U));
    callback2->transitionDone(_componentMock2);
    EXPECT_FALSE(componentInfo2._isTransitionPending);
    EXPECT_EQ(
        100U,
        componentInfo2
            ._transitionTimes[static_cast<uint8_t>(ILifecycleComponent::Transition::Type::INIT)]);
    callback1->transitionDone(_componentMock1);
    EXPECT_FALSE(componentInfo1._isTransitionPending);
    EXPECT_EQ(
        200U,
        componentInfo1
            ._transitionTimes[static_cast<uint8_t>(ILifecycleComponent::Transition::Type::INIT)]);
    EXPECT_CALL(*this, getTimestamp()).InSequence(seq).WillRepeatedly(Return(400U));
    EXPECT_CALL(_componentMock1, getTransitionContext(ILifecycleComponent::Transition::Type::RUN))
        .WillOnce(Return(::async::CONTEXT_INVALID));
    EXPECT_CALL(_componentMock1, startTransition(ILifecycleComponent::Transition::Type::RUN));
    EXPECT_CALL(_componentMock2, getTransitionContext(ILifecycleComponent::Transition::Type::RUN))
        .WillOnce(Return(::async::CONTEXT_INVALID));
    EXPECT_CALL(_componentMock2, startTransition(ILifecycleComponent::Transition::Type::RUN));
    _context.execute();
    Mock::VerifyAndClearExpectations(this);
    EXPECT_TRUE(componentInfo1._isTransitionPending);
    EXPECT_EQ(ILifecycleComponent::Transition::Type::RUN, componentInfo1._lastTransition);
    EXPECT_TRUE(componentInfo2._isTransitionPending);
    EXPECT_EQ(ILifecycleComponent::Transition::Type::RUN, componentInfo2._lastTransition);

    // expect level 3 to be initialized
    EXPECT_CALL(*this, getTimestamp()).InSequence(seq).WillOnce(Return(450U));
    callback1->transitionDone(_componentMock2);
    EXPECT_FALSE(componentInfo2._isTransitionPending);
    EXPECT_EQ(ILifecycleComponent::Transition::Type::RUN, componentInfo2._lastTransition);
    EXPECT_EQ(
        50U,
        componentInfo2
            ._transitionTimes[static_cast<uint8_t>(ILifecycleComponent::Transition::Type::RUN)]);
    _context.execute();
    Mock::VerifyAndClearExpectations(this);

    EXPECT_CALL(*this, getTimestamp()).InSequence(seq).WillOnce(Return(500U));
    callback2->transitionDone(_componentMock1);
    EXPECT_FALSE(componentInfo2._isTransitionPending);
    EXPECT_EQ(ILifecycleComponent::Transition::Type::RUN, componentInfo2._lastTransition);
    EXPECT_EQ(
        100U,
        componentInfo1
            ._transitionTimes[static_cast<uint8_t>(ILifecycleComponent::Transition::Type::RUN)]);

    EXPECT_CALL(
        _listenerMock, lifecycleLevelReached(1U, ILifecycleComponent::Transition::Type::RUN));
    EXPECT_CALL(
        _listenerMock, lifecycleLevelReached(2U, ILifecycleComponent::Transition::Type::RUN));
    EXPECT_CALL(*this, getTimestamp()).InSequence(seq).WillOnce(Return(530U));
    EXPECT_CALL(*this, getTimestamp()).InSequence(seq).WillOnce(Return(531U));
    EXPECT_CALL(*this, getTimestamp()).InSequence(seq).WillOnce(Return(532U));
    EXPECT_CALL(_componentMock3, getTransitionContext(ILifecycleComponent::Transition::Type::INIT))
        .WillOnce(Return(::async::CONTEXT_INVALID));
    EXPECT_CALL(_componentMock3, startTransition(ILifecycleComponent::Transition::Type::INIT));
    _context.execute();
    Mock::VerifyAndClearExpectations(this);

    // expect level 3 to be run after initialization has been done
    EXPECT_CALL(*this, getTimestamp()).InSequence(seq).WillOnce(Return(600U));
    callback3->transitionDone(_componentMock3);
    EXPECT_CALL(*this, getTimestamp()).InSequence(seq).WillOnce(Return(620U));
    EXPECT_CALL(_componentMock3, getTransitionContext(ILifecycleComponent::Transition::Type::RUN))
        .WillOnce(Return(::async::CONTEXT_INVALID));
    EXPECT_CALL(_componentMock3, startTransition(ILifecycleComponent::Transition::Type::RUN));
    _context.execute();

    // Start transition to level 1 even before level 3 has been reached
    cut.transitionToLevel(1U);
    EXPECT_CALL(*this, getTimestamp()).InSequence(seq).WillOnce(Return(680U));
    callback3->transitionDone(_componentMock3);
    EXPECT_CALL(
        _listenerMock, lifecycleLevelReached(3U, ILifecycleComponent::Transition::Type::RUN));
    Mock::VerifyAndClearExpectations(this);

    // expect level 2 to be shutdown
    EXPECT_CALL(*this, getTimestamp()).InSequence(seq).WillOnce(Return(700U));
    EXPECT_CALL(
        _componentMock3, getTransitionContext(ILifecycleComponent::Transition::Type::SHUTDOWN))
        .WillOnce(Return(::async::CONTEXT_INVALID));
    EXPECT_CALL(_componentMock3, startTransition(ILifecycleComponent::Transition::Type::SHUTDOWN));
    _context.execute();
    EXPECT_TRUE(componentInfo3._isTransitionPending);
    EXPECT_EQ(ILifecycleComponent::Transition::Type::SHUTDOWN, componentInfo3._lastTransition);
    Mock::VerifyAndClearExpectations(this);

    EXPECT_CALL(*this, getTimestamp()).InSequence(seq).WillOnce(Return(750U));
    EXPECT_CALL(*this, getTimestamp()).InSequence(seq).WillOnce(Return(751U));
    callback3->transitionDone(_componentMock3);
    EXPECT_FALSE(componentInfo3._isTransitionPending);
    EXPECT_EQ(ILifecycleComponent::Transition::Type::SHUTDOWN, componentInfo3._lastTransition);
    EXPECT_EQ(
        50U,
        componentInfo3._transitionTimes[static_cast<uint8_t>(
            ILifecycleComponent::Transition::Type::SHUTDOWN)]);
    EXPECT_CALL(
        _listenerMock, lifecycleLevelReached(2U, ILifecycleComponent::Transition::Type::SHUTDOWN));
    EXPECT_CALL(
        _listenerMock, lifecycleLevelReached(1U, ILifecycleComponent::Transition::Type::SHUTDOWN));
    _context.execute();
    Mock::VerifyAndClearExpectations(this);

    // Start transition to level 0
    cut.transitionToLevel(0U);
    EXPECT_CALL(*this, getTimestamp()).InSequence(seq).WillOnce(Return(800U));
    EXPECT_CALL(
        _componentMock1, getTransitionContext(ILifecycleComponent::Transition::Type::SHUTDOWN))
        .WillOnce(Return(::async::CONTEXT_INVALID));
    EXPECT_CALL(_componentMock1, startTransition(ILifecycleComponent::Transition::Type::SHUTDOWN));
    ::async::TestContext otherContext(7U);
    otherContext.handleAll();
    EXPECT_CALL(
        _componentMock2, getTransitionContext(ILifecycleComponent::Transition::Type::SHUTDOWN))
        .WillOnce(Return(otherContext.getContext()));
    _context.execute();
    EXPECT_CALL(_componentMock2, startTransition(ILifecycleComponent::Transition::Type::SHUTDOWN));
    otherContext.execute();
    Mock::VerifyAndClearExpectations(this);

    // responses
    EXPECT_CALL(*this, getTimestamp()).InSequence(seq).WillOnce(Return(820U));
    callback1->transitionDone(_componentMock1);
    EXPECT_CALL(*this, getTimestamp()).InSequence(seq).WillOnce(Return(830U));
    callback2->transitionDone(_componentMock2);
    EXPECT_CALL(
        _listenerMock, lifecycleLevelReached(0U, ILifecycleComponent::Transition::Type::SHUTDOWN));
    _context.execute();
    Mock::VerifyAndClearExpectations(this);

    // No initialization when doing next transition to level 1
    cut.transitionToLevel(1U);
    EXPECT_CALL(*this, getTimestamp()).InSequence(seq).WillRepeatedly(Return(400U));
    EXPECT_CALL(_componentMock1, getTransitionContext(ILifecycleComponent::Transition::Type::RUN))
        .WillOnce(Return(::async::CONTEXT_INVALID));
    EXPECT_CALL(_componentMock1, startTransition(ILifecycleComponent::Transition::Type::RUN));
    EXPECT_CALL(_componentMock2, getTransitionContext(ILifecycleComponent::Transition::Type::RUN))
        .WillOnce(Return(::async::CONTEXT_INVALID));
    EXPECT_CALL(_componentMock2, startTransition(ILifecycleComponent::Transition::Type::RUN));
    _context.execute();
    Mock::VerifyAndClearExpectations(this);

    cut.removeLifecycleListener(_listenerMock);
}

TEST_F(LifecycleManagerTest, testAddComponentAssertsComponentsAreAddedWithLevelGreater0)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);

    ::lifecycle::declare::LifecycleManager<4, 4, 4> cut(_context, _getTimestamp);
    NiceMock<LifecycleComponentMock> componentMock;
    ASSERT_THROW({ cut.addComponent("comp1", componentMock, 0U); }, ::estd::assert_exception);
}

TEST_F(LifecycleManagerTest, testAddComponentAssertsNotTooManyComponentsAreAdded)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);

    ::lifecycle::declare::LifecycleManager<3, 4, 4> cut(_context, _getTimestamp);
    NiceMock<LifecycleComponentMock> componentMock1;
    NiceMock<LifecycleComponentMock> componentMock2;
    NiceMock<LifecycleComponentMock> componentMock3;
    NiceMock<LifecycleComponentMock> componentMock4;
    cut.addComponent("comp1", componentMock1, 1U);
    cut.addComponent("comp2", componentMock2, 2U);
    cut.addComponent("comp3", componentMock3, 3U);
    ASSERT_THROW({ cut.addComponent("comp4", componentMock4, 4U); }, ::estd::assert_exception);
}

TEST_F(LifecycleManagerTest, testAddComponentAssertsNotTooManyComponentsPerLevelAreAdded)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);

    ::lifecycle::declare::LifecycleManager<3, 2, 2> cut(_context, _getTimestamp);
    NiceMock<LifecycleComponentMock> componentMock1;
    NiceMock<LifecycleComponentMock> componentMock2;
    NiceMock<LifecycleComponentMock> componentMock3;
    cut.addComponent("comp1", componentMock1, 1U);
    cut.addComponent("comp2", componentMock2, 1U);
    ASSERT_THROW({ cut.addComponent("comp3", componentMock3, 1U); }, ::estd::assert_exception);
}

TEST_F(LifecycleManagerTest, testAddComponentAssertsNotTooManyLevelsAreUsed)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);

    ::lifecycle::declare::LifecycleManager<3, 4, 2> cut(_context, _getTimestamp);
    NiceMock<LifecycleComponentMock> componentMock1;
    NiceMock<LifecycleComponentMock> componentMock2;
    NiceMock<LifecycleComponentMock> componentMock3;
    cut.addComponent("comp1", componentMock1, 1U);
    cut.addComponent("comp2", componentMock2, 4U);
    ASSERT_THROW({ cut.addComponent("comp3", componentMock3, 5U); }, ::estd::assert_exception);
}

TEST_F(LifecycleManagerTest, testAddComponentAssertsComponentsAreAddedByAscendingLevels)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);

    ::lifecycle::declare::LifecycleManager<4, 4, 4> cut(_context, _getTimestamp);
    NiceMock<LifecycleComponentMock> componentMock1;
    NiceMock<LifecycleComponentMock> componentMock2;
    NiceMock<LifecycleComponentMock> componentMock3;
    NiceMock<LifecycleComponentMock> componentMock4;
    cut.addComponent("comp1", componentMock1, 1U);
    cut.addComponent("comp2", componentMock2, 1U);
    cut.addComponent("comp3", componentMock3, 2U);
    ASSERT_THROW({ cut.addComponent("comp4", componentMock4, 1U); }, ::estd::assert_exception);
}

} // anonymous namespace
