// Copyright 2024 Accenture.

#pragma once

#include "async/Types.h"

#include <util/string/ConstString.h>

#include <estd/forward_list.h>

#define FUNCTION_EXECUTION_MONITOR_CONCAT_INTERNAL(x, y) x##y
#define FUNCTION_EXECUTION_MONITOR_CONCAT(x, y)          FUNCTION_EXECUTION_MONITOR_CONCAT_INTERNAL(x, y)

#define FUNCTION_EXECUTION_POINT(_runtimeMonitorType, _name)               \
    static ::runtime::FunctionExecutionMonitor<_runtimeMonitorType>::Point \
        FUNCTION_EXECUTION_MONITOR_CONCAT(_fep_, __LINE__)(_name);         \
    const ::runtime::FunctionExecutionMonitor<_runtimeMonitorType>::Scope  \
    FUNCTION_EXECUTION_MONITOR_CONCAT(                                     \
        _scope_, __LINE__)(FUNCTION_EXECUTION_MONITOR_CONCAT(_fep_, __LINE__))

namespace runtime
{

template<class RuntimeMonitor>
class FunctionExecutionMonitor
{
public:
    struct Point final : public ::estd::forward_list_node<Point>
    {
        explicit Point(char const* name);
        ~Point();

        char const* _name;
        typename RuntimeMonitor::FunctionEntryType _entry;
        typename RuntimeMonitor::FunctionStatisticsType _snapshot;
    };

    using PointListType = ::estd::forward_list<Point>;

    class Scope final
    {
    public:
        explicit Scope(Point& point);
        ~Scope();

    private:
        Point* _point;
    };

    class Iterator
    {
    public:
        using StatisticsType = typename RuntimeMonitor::FunctionStatisticsType;

        Iterator();

        void next();
        void reset();
        bool hasValue() const;
        char const* getName() const;
        StatisticsType getStatistics() const;

    private:
        typename PointListType::iterator _it;
    };

    static void init(RuntimeMonitor& runtimeMonitor);
    static void takeSnapshot();
    static void setIsUsed();
    static bool isUsed();

private:
    friend class Iterator;
    friend struct Point;
    friend class Scope;

    static PointListType _points;
    static RuntimeMonitor* _runtimeMonitor;
    static bool _isUsed;
};

template<class RuntimeMonitor>
typename FunctionExecutionMonitor<RuntimeMonitor>::PointListType
    FunctionExecutionMonitor<RuntimeMonitor>::_points;
template<class RuntimeMonitor>
RuntimeMonitor* FunctionExecutionMonitor<RuntimeMonitor>::_runtimeMonitor = nullptr;
template<class RuntimeMonitor>
bool FunctionExecutionMonitor<RuntimeMonitor>::_isUsed = false;

template<class RuntimeMonitor>
class FunctionExecutionMonitorUsage
{
public:
    static void setIsUsed();

private:
    struct Initializer
    {
    public:
        Initializer();

        void setIsUsed();
    };

    static Initializer _initializer;
};
template<class RuntimeMonitor>
typename FunctionExecutionMonitorUsage<RuntimeMonitor>::Initializer
    FunctionExecutionMonitorUsage<RuntimeMonitor>::_initializer;

template<class RuntimeMonitor>
void FunctionExecutionMonitor<RuntimeMonitor>::init(RuntimeMonitor& runtimeMonitor)
{
    _runtimeMonitor = &runtimeMonitor;
}

template<class RuntimeMonitor>
void FunctionExecutionMonitor<RuntimeMonitor>::takeSnapshot()
{
    ::async::LockType const lock;
    for (Point& point : _points)
    {
        point._snapshot = point._entry;
        point._entry.reset();
    }
}

template<class RuntimeMonitor>
void FunctionExecutionMonitor<RuntimeMonitor>::setIsUsed()
{
    _isUsed = true;
}

template<class RuntimeMonitor>
bool FunctionExecutionMonitor<RuntimeMonitor>::isUsed()
{
    return _isUsed;
}

template<class RuntimeMonitor>
FunctionExecutionMonitor<RuntimeMonitor>::Point::Point(char const* const name)
: _name(name), _entry(), _snapshot()
{
    FunctionExecutionMonitorUsage<RuntimeMonitor>::setIsUsed();
    ::util::string::ConstString const findName(name);
    ::async::LockType const lock;
    auto prevIt = _points.before_begin();
    for (Point const& point : _points)
    {
        if (findName <= ::util::string::ConstString(point._name))
        {
            break;
        }
        ++prevIt;
    }
    (void)_points.insert_after(prevIt, *this);
}

template<class RuntimeMonitor>
FunctionExecutionMonitor<RuntimeMonitor>::Point::~Point()
{
    ::async::LockType const lock;
    _points.remove(*this);
}

template<class RuntimeMonitor>
FunctionExecutionMonitor<RuntimeMonitor>::Scope::Scope(Point& point) : _point(nullptr)
{
    if (FunctionExecutionMonitor::_runtimeMonitor != nullptr)
    {
        FunctionExecutionMonitor::_runtimeMonitor->enterFunction(point._entry);
        _point = &point;
    }
}

template<class RuntimeMonitor>
FunctionExecutionMonitor<RuntimeMonitor>::Scope::~Scope()
{
    if (_point != nullptr)
    {
        FunctionExecutionMonitor::_runtimeMonitor->leaveFunction(_point->_entry);
    }
}

template<class RuntimeMonitor>
FunctionExecutionMonitor<RuntimeMonitor>::Iterator::Iterator()
{
    reset();
}

template<class RuntimeMonitor>
void FunctionExecutionMonitor<RuntimeMonitor>::Iterator::next()
{
    ::async::LockType const lock;
    ++_it;
}

template<class RuntimeMonitor>
void FunctionExecutionMonitor<RuntimeMonitor>::Iterator::reset()
{
    _it = FunctionExecutionMonitor<RuntimeMonitor>::_points.begin();
}

template<class RuntimeMonitor>
bool FunctionExecutionMonitor<RuntimeMonitor>::Iterator::hasValue() const
{
    return _it != FunctionExecutionMonitor<RuntimeMonitor>::_points.end();
}

template<class RuntimeMonitor>
char const* FunctionExecutionMonitor<RuntimeMonitor>::Iterator::getName() const
{
    return _it->_name;
}

template<class RuntimeMonitor>
typename RuntimeMonitor::FunctionStatisticsType
FunctionExecutionMonitor<RuntimeMonitor>::Iterator::getStatistics() const
{
    return _it->_snapshot;
}

template<class RuntimeMonitor>
void FunctionExecutionMonitorUsage<RuntimeMonitor>::setIsUsed()
{
    _initializer.setIsUsed();
}

template<class RuntimeMonitor>
FunctionExecutionMonitorUsage<RuntimeMonitor>::Initializer::Initializer()
{
    setIsUsed();
}

template<class RuntimeMonitor>
void FunctionExecutionMonitorUsage<RuntimeMonitor>::Initializer::setIsUsed()
{
    FunctionExecutionMonitor<RuntimeMonitor>::setIsUsed();
}

} // namespace runtime
