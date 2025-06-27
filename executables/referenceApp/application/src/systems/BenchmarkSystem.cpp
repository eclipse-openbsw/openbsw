// Copyright 2024 Accenture.

#include "systems/BenchmarkSystem.h"

#include "console/console.h"
#include "logger/logger.h"

#include <async/FutureSupport.h>
#include <bsp/SystemTime.h>
#include <outputManager/Output.h>
#include <outputPwm/OutputPwm.h>

#define FTM4 IP_FTM4

static uint32_t interruptLatencyStart = 0;
static uint32_t interruptLatencyEnd   = 0;

using bios::Output;
using bios::OutputPwm;

extern "C"
{
void FTM4_Ch0_Ch1_IRQHandler()
{
    interruptLatencyEnd = DWT->CYCCNT;
    SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;
    FTM4->CONTROLS[1].CnSC &= ~FTM_CnSC_CHF_MASK;
    FTM4->CONTROLS[1].CnSC &= ~FTM_CnSC_CHIE_MASK;
    OutputPwm::setDuty(OutputPwm::EVAL_LED_RED_PWM, 0);
}
}

namespace
{
constexpr uint32_t SYSTEM_CYCLE_TIME  = 2000; /* milliseconds */
constexpr uint32_t LOAD_COUNTER_LIMIT = 0x1800;
} // namespace

namespace systems
{

BenchmarkSystem::BenchmarkSystem(
    ::async::ContextType const context, ::async::AsyncBinding::RuntimeMonitorType& runtimeMonitor)
: _context(context)
, _taskSwitchLatencyStart(0)
, _taskSwitchLatencyEnd(0)
, _taskSwitchAfterTimeoutLatencyStart(0)
, _taskSwitchAfterTimeoutLatencyEnd(0)
, _future(0)
, _eventWaiterRunnable(_future, _taskSwitchLatencyEnd)
, _eventSetterRunnable(_future, _taskSwitchLatencyStart)
, _timeoutWaiterRunnable(_taskSwitchAfterTimeoutLatencyEnd)
, _sysadminLoadRunnable(LOAD_COUNTER_LIMIT)
, _canLoadRunnable(LOAD_COUNTER_LIMIT * 2)
, _demoLoadRunnable(LOAD_COUNTER_LIMIT * 4)
, _udsLoadRunnable(LOAD_COUNTER_LIMIT * 10)
, _bgLoadRunnable(LOAD_COUNTER_LIMIT * 20)
, _taskSwitchLatencyTestRunning(false)
, _taskSwitchAfterTimeoutLatencyTestRunning(false)
, _interruptLatencyTestRunning(false)
, _loadTestRunning(false)
, _runtimeMonitorRunning(false)
, _runtimeMonitor(runtimeMonitor)
{
    setTransitionContext(context);
}

void BenchmarkSystem::init() { transitionDone(); }

void BenchmarkSystem::run()
{
    if (DWT_CTRL_CYCCNTENA_Msk != (DWT_CTRL_CYCCNTENA_Msk & DWT->CTRL))
    {
        /* enable DWT cyclic counter */
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    }
    Logger::info(BENCH, "--------------------------------------------------");
    Logger::info(BENCH, "Benchmark tests are starting now.");
    Logger::info(BENCH, "Please ensure the system is in the expected state.");
    Logger::info(BENCH, "--------------------------------------------------\n");
    _runtimeMonitor.start();
    _runtimeMonitorRunning = true;
    ::async::scheduleAtFixedRate(
        _context, *this, _timeout, SYSTEM_CYCLE_TIME, ::async::TimeUnit::MILLISECONDS);
    transitionDone();
}

void BenchmarkSystem::shutdown() { transitionDone(); }

void BenchmarkSystem::execute()
{
    if (_runtimeMonitorRunning)
    {
        startLoadTest();
    }
    else if (_loadTestRunning)
    {
        handleLoadTest();
        ::logger::run();
        ::console::run();
        if (!_loadTestRunning)
        {
            _runtimeMonitor.stop();
            startInterruptLatencyTest();
        }
    }
    else if (_interruptLatencyTestRunning)
    {
        handleInterruptLatencyTest();
        ::logger::run();
        ::console::run();
        if (!_interruptLatencyTestRunning)
        {
            startTaskSwitchLatencyTest();
        }
    }
    else if (_taskSwitchLatencyTestRunning)
    {
        handleTaskSwitchLatencyTest();
        ::logger::run();
        ::console::run();
        if (!_taskSwitchLatencyTestRunning)
        {
            startTaskSwitchAfterTimeoutLatencyTest();
        }
    }
    else if (_taskSwitchAfterTimeoutLatencyTestRunning)
    {
        handleTaskSwitchAfterTimeoutLatencyTest();
        if (!_taskSwitchAfterTimeoutLatencyTestRunning)
        {
            Logger::info(BENCH, "---------------------------------------");
            Logger::info(BENCH, "Benchmark tests completed successfully.");
            Logger::info(BENCH, "Review the results for analysis.");
            Logger::info(BENCH, "---------------------------------------");
            ::logger::run();
            ::console::run();
        }
    }
    else
    {
        ::logger::run();
        ::console::run();
    }
}

void BenchmarkSystem::startInterruptLatencyTest()
{
    _interruptLatencyTestRunning = true;
    interruptLatencyStart        = 0;
    interruptLatencyEnd          = 0;
    OutputPwm::setDuty(OutputPwm::EVAL_LED_RED_PWM, 1000);

    /**
     * The FTM timer is started, initiating the generation of a PWM signal.
     * This code waits until the interrupt flag is set, the start timestamp is recorded,
     * and the corresponding interrupt is enabled in the FTM.
     * The end timestamp is recorded when the ISR is entered.
     * Note: systick interrupt is disabled until the FTM interrupt ISR is called
     */
    do
    {
        if (FTM4->CONTROLS[1].CnSC & FTM_CnSC_CHF_MASK)
        {
            SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;
            interruptLatencyStart = DWT->CYCCNT;
            FTM4->CONTROLS[1].CnSC |= FTM_CnSC_CHIE_MASK;
            break;
        }
    } while (true);
}

void BenchmarkSystem::handleInterruptLatencyTest()
{
    if (interruptLatencyStart != 0 && interruptLatencyEnd != 0)
    {
        uint32_t cycles = calculateTimeDifference(interruptLatencyStart, interruptLatencyEnd);
        double latency  = (static_cast<uint64_t>(cycles) * 1000000000) / configCPU_CLOCK_HZ;
        Logger::info(
            BENCH,
            "Interrupt latency: %llu ns (%u cycles)\n",
            static_cast<uint64_t>(latency),
            cycles);
        _interruptLatencyTestRunning = false;
    }
}

void BenchmarkSystem::startTaskSwitchLatencyTest()
{
    _taskSwitchLatencyTestRunning = true;
    _taskSwitchLatencyStart       = 0;
    _taskSwitchLatencyEnd         = 0;
    ::async::execute(TASK_SYSADMIN, _eventWaiterRunnable);
    ::async::execute(TASK_CAN, _eventSetterRunnable);
}

void BenchmarkSystem::handleTaskSwitchLatencyTest()
{
    if (_taskSwitchLatencyStart != 0 && _taskSwitchLatencyEnd != 0)
    {
        uint32_t cycles = calculateTimeDifference(_taskSwitchLatencyStart, _taskSwitchLatencyEnd);
        double latency  = (static_cast<uint64_t>(cycles) * 1000000000) / configCPU_CLOCK_HZ;
        Logger::info(
            BENCH,
            "Task switch latency: %llu ns (%u cycles)\n",
            static_cast<uint64_t>(latency),
            cycles);
        _taskSwitchLatencyTestRunning = false;
    }
}

void BenchmarkSystem::startTaskSwitchAfterTimeoutLatencyTest()
{
    _taskSwitchAfterTimeoutLatencyTestRunning = true;
    _taskSwitchAfterTimeoutLatencyEnd         = 0;
    _taskSwitchAfterTimeoutLatencyStart       = DWT->CYCCNT;
    ::async::schedule(
        TASK_SYSADMIN,
        _timeoutWaiterRunnable,
        _taskSwitchTimeout,
        100,
        ::async::TimeUnit::MILLISECONDS);
}

void BenchmarkSystem::handleTaskSwitchAfterTimeoutLatencyTest()
{
    if (_taskSwitchAfterTimeoutLatencyStart != 0 && _taskSwitchAfterTimeoutLatencyEnd != 0)
    {
        uint32_t cycles = calculateTimeDifference(
            _taskSwitchAfterTimeoutLatencyStart, _taskSwitchAfterTimeoutLatencyEnd);
        double latency = (static_cast<uint64_t>(cycles) * 1000000000) / configCPU_CLOCK_HZ;
        Logger::info(
            BENCH,
            "Task switch after timeout latency: %llu ns (%u cycles)\n",
            static_cast<uint64_t>(latency),
            cycles);
        _taskSwitchAfterTimeoutLatencyTestRunning = false;
    }
}

void BenchmarkSystem::startLoadTest()
{
    TaskStatistics taskStatistics;

    _loadTestRunning = true;

    taskStatistics.copyFrom(_runtimeMonitor.getTaskStatistics());
    uint32_t totalRuntime  = _runtimeMonitor.reset();
    _runtimeMonitorRunning = false;
    uint32_t _idlePercentage
        = getPercentage(taskStatistics.getStatistics(0).getTotalRuntime(), totalRuntime);
    Logger::info(
        BENCH,
        "Load test: CPU idle: %d.%02d %% (NO load)",
        _idlePercentage / 100U,
        _idlePercentage % 100U);

    ::async::scheduleAtFixedRate(
        TASK_SYSADMIN, _sysadminLoadRunnable, _sysadminTimeout, 5, ::async::TimeUnit::MILLISECONDS);
    ::async::scheduleAtFixedRate(
        TASK_CAN, _canLoadRunnable, _canTimeout, 10, ::async::TimeUnit::MILLISECONDS);
    ::async::scheduleAtFixedRate(
        TASK_DEMO, _demoLoadRunnable, _demoTimeout, 20, ::async::TimeUnit::MILLISECONDS);
    ::async::scheduleAtFixedRate(
        TASK_UDS, _udsLoadRunnable, _udsTimeout, 50, ::async::TimeUnit::MILLISECONDS);
    ::async::scheduleAtFixedRate(
        TASK_BACKGROUND, _bgLoadRunnable, _bgTimeout, 100, ::async::TimeUnit::MILLISECONDS);
}

void BenchmarkSystem::handleLoadTest()
{
    TaskStatistics taskStatistics;

    taskStatistics.copyFrom(_runtimeMonitor.getTaskStatistics());
    uint32_t totalRuntime = _runtimeMonitor.reset();
    uint32_t _idlePercentage
        = getPercentage(taskStatistics.getStatistics(0).getTotalRuntime(), totalRuntime);
    _sysadminTimeout.cancel();
    _canTimeout.cancel();
    _demoTimeout.cancel();
    _udsTimeout.cancel();
    _bgTimeout.cancel();

    Logger::info(
        BENCH,
        "Load test: CPU idle: %d.%02d %% (load) (LOAD_COUNTER_LIMIT: 0x%x)\n",
        _idlePercentage / 100U,
        _idlePercentage % 100U,
        LOAD_COUNTER_LIMIT);

    _loadTestRunning = false;
}

uint32_t BenchmarkSystem::getPercentage(uint64_t value, uint64_t total)
{
    return (total != 0U) ? static_cast<uint32_t>(
               static_cast<uint64_t>(value) * 10000U / static_cast<uint64_t>(total))
                         : 0U;
}

uint32_t BenchmarkSystem::calculateTimeDifference(uint32_t start, uint32_t end)
{
    return (end >= start) ? (end - start) : (end + (0xFFFFFFFF - start + 1));
}

} // namespace systems
