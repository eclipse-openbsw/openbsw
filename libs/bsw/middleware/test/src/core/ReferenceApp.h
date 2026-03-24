#pragma once

#include <etl/expected.h>
#include <gmock/gmock.h>

#include "middleware/core/Future.h"

namespace middleware
{
namespace core
{
namespace test
{

template<typename ArgType>
class RefApp
{
public:
    MOCK_METHOD(void, methodCallback, ((etl::expected<ArgType, Future::State> const&)));
    MOCK_METHOD(void, eventCallback, (ArgType const&));
};

} // namespace test
} // namespace core
} // namespace middleware
