#include <cstdint>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "middleware/core/FutureDispatcher.h"
#include "middleware/core/SkeletonAttribute.h"
#include "middleware/core/SkeletonBase.h"
#include "middleware/core/types.h"

namespace middleware::core::test
{

namespace internal
{
constexpr uint16_t event_id = 0U;
} // namespace internal

using ArgType                  = uint8_t;
using GetterFutureTraitsPolicy = DispatcherTraits<ArgType, 0U, false>;

class DerivedSkeleton : public ::middleware::core::SkeletonBase
{
public:
    MOCK_METHOD(uint16_t, getServiceId, (), (const, override));
    MOCK_METHOD(HRESULT, onNewMessageReceived, (Message const&), (override));

    DerivedSkeleton() : SkeletonBase(), attribute(*this) {}

    SkeletonAttribute<ArgType, internal::event_id, true> attribute;
};

class SkeletonAttributeTest : public ::testing::Test
{
public:
    void SetUp() override {}

    void TearDown() override {}
};

TEST_F(SkeletonAttributeTest, SendPayload)
{
    DerivedSkeleton skeleton;
    ArgType const payload = 0xFFU;

    EXPECT_EQ(skeleton.attribute.send(payload), HRESULT::NotRegistered);

    skeleton.attribute.set(skeleton.attribute.get());
    EXPECT_EQ(skeleton.attribute.send(), HRESULT::NotRegistered);
}

} // namespace middleware::core::test
