// Copyright 2024 Accenture.

#pragma once

#include <platform/estdint.h>

namespace docan
{
/**
 * JobHandle consisting of 2 IDs: counter and user data. User data is used for uniquely identifying
 * the job it belongs to. Counter is used for separating the same job IDs in a certain time frame.
 */
template<typename Counter, typename UserData>
class DoCanJobHandle
{
public:
    using CounterType  = Counter;
    using UserDataType = UserData;

    DoCanJobHandle(CounterType const counter, UserDataType const userData)
    : _counter(counter), _userData(userData)
    {}

    DoCanJobHandle() : DoCanJobHandle(static_cast<CounterType>(0), static_cast<UserDataType>(0)) {}

    bool operator==(DoCanJobHandle const& other) const
    {
        return (_counter == other._counter) && (_userData == other._userData);
    }

    bool operator!=(DoCanJobHandle const& other) const { return !(*this == other); }

    DoCanJobHandle(DoCanJobHandle const& other)            = default;
    DoCanJobHandle(DoCanJobHandle&& other)                 = default;
    DoCanJobHandle& operator=(DoCanJobHandle const& other) = default;
    DoCanJobHandle& operator=(DoCanJobHandle&& other)      = default;

private:
    CounterType _counter;
    UserDataType _userData;
};
} // namespace docan

