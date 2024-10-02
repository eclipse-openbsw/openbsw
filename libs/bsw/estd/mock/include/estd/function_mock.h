// Copyright 2024 Accenture.

#ifndef GUARD_F2D205E8_1FBD_44A5_A199_4E62CDF32712
#define GUARD_F2D205E8_1FBD_44A5_A199_4E62CDF32712

#include <estd/functional.h>

#include <gmock/gmock.h>

namespace estd
{

template<typename>
struct function_mock;

template<typename R, typename... Ps>
struct function_mock<R(Ps...)>
{
    using self_type = function_mock<R(Ps...)>;

    MOCK_METHOD0(callee, void());

    operator ::estd::function<R(Ps...)>()
    {
        return ::estd::function<R(Ps...)>::template create<self_type, &self_type::callee>(*this);
    }
};

} // namespace estd

#endif /* GUARD_F2D205E8_1FBD_44A5_A199_4E62CDF32712 */
