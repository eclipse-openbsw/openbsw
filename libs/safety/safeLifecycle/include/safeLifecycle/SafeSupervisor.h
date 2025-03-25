// Copyright 2024 Accenture.

#pragma once

#include <safeMonitor/Sequence.h>
#include <safeMonitor/Trigger.h>
#include <safeMonitor/Value.h>

#include <platform/estdint.h>

namespace safety
{

class SafeSupervisor
{
public:
    enum class Event : uint8_t
    {
        SAFE_EVENT_DUMMY,
        SAFETY_MANAGER_SEQUENCE_DEVIATION,
        WATCHDOG_STARTUP_CHECK_FAILURE,
        SAFE_WATCHDOG_SEQUENCE_DEVIATION,
        SAFE_WATCHDOG_CONFIGURATION_ERROR,
        SAFE_WATCHDOG_SERVICE_DEVIATION,
        MPU_UNLOCKED_ON_SAFETY_MANAGER_ENTRY,
        MPU_LOCKED_ON_SAFETY_MANAGER_EXIT
    };

    enum class SafetyManagerSequence
    {
        SAFETY_MANAGER_ENTER,
        SAFETY_MANAGER_LEAVE
    };

    enum class EnterLeaveSequence
    {
        ENTER,
        LEAVE
    };

    using SafetyManagerSequenceMonitor
        = ::safeMonitor::Sequence<SafeSupervisor, Event, SafetyManagerSequence>;
    using TriggerMonitor  = ::safeMonitor::Trigger<SafeSupervisor, Event>;
    using SequenceMonitor = ::safeMonitor::Sequence<SafeSupervisor, Event, EnterLeaveSequence>;
    using ValueMonitor    = ::safeMonitor::Value<SafeSupervisor, Event, bool>;

    SafeSupervisor();

    void init() const;
    void handle(Event const& event);
    static SafeSupervisor& getInstance();
    void enterLimpHome();
    void leaveLimpHome();
    bool limpHome() const;

    SafetyManagerSequenceMonitor safetyManagerSequenceMonitor;
    TriggerMonitor watchdogStartupCheckMonitor;
    SequenceMonitor safeWatchdogSequenceMonitor;
    ValueMonitor safeWatchdogConfigMonitor;
    TriggerMonitor serviceWatchdogMonitor;
    ValueMonitor mpuStatusCheckOnEnterMonitor;
    ValueMonitor mpuStatusCheckOnLeaveMonitor;

private:
    bool _limpHome;
};

} // namespace safety
