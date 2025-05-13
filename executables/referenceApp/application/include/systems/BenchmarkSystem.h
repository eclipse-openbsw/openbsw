// Copyright 2024 Accenture.

#pragma once

#include "app/BenchmarkLogger.h"

#include <async/AsyncBinding.h>
#include <async/FutureSupport.h>
#include <console/AsyncCommandWrapper.h>
#include <lifecycle/AsyncLifecycleComponent.h>
#include <lifecycle/console/LifecycleControlCommand.h>
#include <mcu/mcu.h>
#include <runtime/StatisticsContainer.h>

using ::util::logger::BENCH;
using ::util::logger::Logger;

namespace systems
{
class BenchmarkSystem
: public ::lifecycle::AsyncLifecycleComponent
, private ::async::IRunnable
{
public:
    explicit BenchmarkSystem(
        ::async::ContextType context, ::async::AsyncBinding::RuntimeMonitorType& runtimeMonitor);

    BenchmarkSystem(BenchmarkSystem const&)            = delete;
    BenchmarkSystem& operator=(BenchmarkSystem const&) = delete;

    void init() override;
    void run() override;
    void shutdown() override;

    void cyclic();

private:
    void execute() override;

    uint32_t getPercentage(uint64_t value, uint64_t total);
    void startTaskSwitchLatencyTest();
    void handleTaskSwitchLatencyTest();
    void startTaskSwitchAfterTimeoutLatencyTest();
    void handleTaskSwitchAfterTimeoutLatencyTest();
    void startInterruptLatencyTest();
    void handleInterruptLatencyTest();
    void startLoadTest();
    void handleLoadTest();
    uint32_t calculateTimeDifference(uint32_t start, uint32_t end);

private:
    using TaskStatistics = ::runtime::declare::StatisticsContainer<
        ::runtime::RuntimeStatistics,
        ::async::AsyncBindingType::AdapterType::FREERTOS_TASK_COUNT>;

    class EventWaiterRunnable : public ::async::IRunnable
    {
    public:
        explicit EventWaiterRunnable(::async::FutureSupport& future, uint32_t& latencyEnd)
        : _future(future), _timestamp(latencyEnd)
        {}

        void execute() override
        {
            _future.wait();
            _timestamp = DWT->CYCCNT; /* HW cycle counter */
        }

    private:
        ::async::FutureSupport& _future;
        uint32_t& _timestamp;
    };

    class EventSetterRunnable : public ::async::IRunnable
    {
    public:
        explicit EventSetterRunnable(::async::FutureSupport& future, uint32_t& latencyStart)
        : _future(future), _timestamp(latencyStart)
        {}

        void execute() override
        {
            _timestamp = DWT->CYCCNT;
            _future.notify();
        }

    private:
        ::async::FutureSupport& _future;
        uint32_t& _timestamp;
    };

    class TimoutWaiterRunnable : public ::async::IRunnable
    {
    public:
        explicit TimoutWaiterRunnable(uint32_t& latencyEnd) : _timestamp(latencyEnd) {}

        void execute() override { _timestamp = DWT->CYCCNT; }

    private:
        uint32_t& _timestamp;
    };

    class LoadRunnable : public ::async::IRunnable
    {
    public:
        explicit LoadRunnable(uint32_t limit) : _limit(limit) {}

        void execute() override
        {
            uint32_t volatile counter = 0;
            while (counter++ < _limit) {}
        }

    private:
        uint32_t _limit;
    };
    friend class EventWaiterRunnable;
    friend class EventSetterRunnable;
    friend class TimoutWaiterRunnable;
    ::async::ContextType const _context;
    uint32_t _taskSwitchLatencyStart;
    uint32_t _taskSwitchLatencyEnd;
    uint32_t _taskSwitchAfterTimeoutLatencyStart;
    uint32_t _taskSwitchAfterTimeoutLatencyEnd;
    ::async::TimeoutType _timeout;
    ::async::TimeoutType _taskSwitchTimeout;
    ::async::TimeoutType _sysadminTimeout;
    ::async::TimeoutType _canTimeout;
    ::async::TimeoutType _demoTimeout;
    ::async::TimeoutType _udsTimeout;
    ::async::TimeoutType _bgTimeout;
    ::async::FutureSupport _future;
    EventWaiterRunnable _eventWaiterRunnable;
    EventSetterRunnable _eventSetterRunnable;
    TimoutWaiterRunnable _timeoutWaiterRunnable;
    LoadRunnable _sysadminLoadRunnable;
    LoadRunnable _canLoadRunnable;
    LoadRunnable _demoLoadRunnable;
    LoadRunnable _udsLoadRunnable;
    LoadRunnable _bgLoadRunnable;
    bool _taskSwitchLatencyTestRunning;
    bool _taskSwitchAfterTimeoutLatencyTestRunning;
    bool _interruptLatencyTestRunning;
    bool _loadTestRunning;
    bool _runtimeMonitorRunning;
    ::async::AsyncBinding::RuntimeMonitorType& _runtimeMonitor;
};

} // namespace systems
