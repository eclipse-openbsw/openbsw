// Copyright 2024 Accenture.

#include "lifecycle/LifecycleManagerInitializer.h"

#include "lifecycle/LifecycleComponentMock.h"

#include <async/AsyncMock.h>
#include <async/TestContext.h>

#include <gtest/esr_extensions.h>

namespace
{
using namespace ::lifecycle;
using ::testing::_;
using ::testing::Return;
using ::testing::SaveRef;
using ::testing::WithArg;

class LifecycleManagerInitializerTest : public ::testing::Test
{
protected:
    static size_t const NUM_COMPONENTS = 21U;

    LifecycleManagerInitializerTest()
    : _asyncMock()
    , _components()
    , _callbacks{nullptr}
    , _getTimestamp(LifecycleManager::GetTimestampType::create<
                    LifecycleManagerInitializerTest,
                    &LifecycleManagerInitializerTest::getTimestamp>(*this))
    , _context(1U)
    {}

    void SetUp() override { _context.handleAll(); }

    uint32_t getTimestamp() const { return 0U; }

    void
    expectTransition(size_t start, size_t size, ILifecycleComponent::Transition::Type transition)
    {
        for (size_t i = start; i < (start + size); ++i)
        {
            EXPECT_CALL(_components[i], getTransitionContext(transition))
                .WillOnce(Return(::async::CONTEXT_INVALID));
        }

        for (size_t i = start; i < (start + size); ++i)
        {
            EXPECT_CALL(_components[i], startTransition(transition));
        }
    }

    void transitionDone(size_t start, size_t size)
    {
        for (size_t i = start; i < (start + size); ++i)
        {
            _callbacks[i]->transitionDone(_components[i]);
        }
    }

    ::testing::StrictMock<::async::AsyncMock> _asyncMock;
    ::testing::StrictMock<LifecycleComponentMock> _components[NUM_COMPONENTS];
    ILifecycleComponentCallback* _callbacks[NUM_COMPONENTS];
    LifecycleManager::GetTimestampType _getTimestamp;
    ::async::TestContext _context;
};

size_t const LifecycleManagerInitializerTest::NUM_COMPONENTS;

TEST_F(LifecycleManagerInitializerTest, createManager)
{
    ::testing::InSequence inSequence;
    for (size_t i = 0U; i < NUM_COMPONENTS; ++i)
    {
        EXPECT_CALL(_components[i], initCallback(_))
            .WillOnce(WithArg<0>(SaveRef<0>(&_callbacks[i])));
    }

    uint8_t storedLevel = 0U;

    auto const level1 = makeLevel( // level 1, const definition
        makeComponent("comp0", _components[0U]),
        makeComponent("comp1", _components[1U]));

    LifecycleManager& manager = createLifecycleManager(
        _context,
        _getTimestamp,
        level1,
        makeLevel( // level 2, inline definition
            makeComponent("comp2", _components[2U]),
            makeComponent("comp3", _components[3U]),
            makeComponent("comp4", _components[4U]),
            makeComponent("comp5", _components[5U]),
            makeComponent("comp6", _components[6U]),
            makeComponent("comp7", _components[7U]),
            makeComponent("comp8", _components[8U]),
            makeComponent("comp9", _components[9U]),
            makeComponent("comp10", _components[10U]),
            makeComponent("comp11", _components[11U]),
            makeComponent("comp12", _components[12U]),
            makeComponent("comp13", _components[13U]),
            makeComponent("comp14", _components[14U]),
            makeComponent("comp15", _components[15U])),
        // store level 2
        storeLevel(storedLevel),
        makeLevel( // level 3, inline definition
            makeComponent("comp16", _components[16U]),
            makeComponent("comp17", _components[17U]),
            makeComponent("comp18", _components[18U]),
            makeComponent("comp19", _components[19U]),
            makeComponent("comp20", _components[20U])));

    ASSERT_EQ(2U, storedLevel);
    ASSERT_EQ(3U, manager.getLevelCount());
    ASSERT_EQ(NUM_COMPONENTS, manager.getComponentCount());

    for (size_t i = 0U; i < NUM_COMPONENTS; ++i)
    {
        ASSERT_EQ(&_components[i], manager.getComponentInfo(i)._component);
    }

    // check if components are at their appropriate levels

    manager.transitionToLevel(1U);

    expectTransition(0, 2, ILifecycleComponent::Transition::Type::INIT);
    _context.execute();

    expectTransition(0, 2, ILifecycleComponent::Transition::Type::RUN);
    transitionDone(0, 2);
    _context.execute();

    transitionDone(0, 2);
    _context.execute();

    manager.transitionToLevel(2U);

    expectTransition(2, 14, ILifecycleComponent::Transition::Type::INIT);
    _context.execute();

    expectTransition(2, 14, ILifecycleComponent::Transition::Type::RUN);
    transitionDone(2, 14);
    _context.execute();

    transitionDone(2, 14);
    _context.execute();

    manager.transitionToLevel(3U);

    expectTransition(16, 5, ILifecycleComponent::Transition::Type::INIT);
    _context.execute();

    expectTransition(16, 5, ILifecycleComponent::Transition::Type::RUN);
    transitionDone(16, 5);
    _context.execute();

    transitionDone(16, 5);
    _context.execute();
}
} // namespace
