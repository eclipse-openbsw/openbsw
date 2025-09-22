// Copyright 2024 Accenture.

#pragma once

#include "safeSupervisor/utils/explicitly_constructible.h"

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
        MPU_LOCKED_ON_SAFETY_MANAGER_EXIT,
        INTEGRITY_EVENT_DMA_ECC_ERROR,
        ROM_CHECK_FAILURE,
        MPU_NOT_ENABLED,
        MPU_CONFIGURATION_ERROR,
        ADC_REFERENCE_ERROR,
        INTERNAL_SUPPLIES_ERROR
    };

    enum class SafetyManagerSequence : uint8_t
    {
        SAFETY_MANAGER_ENTER,
        SAFETY_MANAGER_LEAVE
    };

    enum class EnterLeaveSequence : uint8_t
    {
        ENTER,
        LEAVE
    };

    using SafetyManagerSequenceMonitor
        = ::safeMonitor::Sequence<SafeSupervisor, Event, SafetyManagerSequence>;
    using TriggerMonitor
        = ::safeMonitor::Trigger<SafeSupervisor, Event, ::safeMonitor::DefaultMutex, uint32_t>;
    using SequenceMonitor = ::safeMonitor::Sequence<SafeSupervisor, Event, EnterLeaveSequence>;
    using ValueMonitor    = ::safeMonitor::Value<SafeSupervisor, Event, bool>;

    SafeSupervisor();

    void init() const;
    void handle(Event const& event);
    static SafeSupervisor& getInstance();

    void enterLimpHome() { _limpHome = true; }

    void leaveLimpHome() { _limpHome = false; }

    bool inLimpHomeState() const { return _limpHome; }

    SafetyManagerSequenceMonitor safetyManagerSequenceMonitor;
    TriggerMonitor watchdogStartupCheckMonitor;
    SequenceMonitor safeWatchdogSequenceMonitor;
    ValueMonitor safeWatchdogConfigMonitor;
    TriggerMonitor serviceWatchdogMonitor;
    ValueMonitor mpuStatusCheckOnEnterMonitor;
    ValueMonitor mpuStatusCheckOnLeaveMonitor;
    TriggerMonitor dmaEccMonitor;
    TriggerMonitor romCheckMonitor;
    TriggerMonitor mpuEnableMonitor;
    TriggerMonitor mpuConfigMonitor;
    TriggerMonitor adcReferenceMonitor;
    TriggerMonitor internalSuppliesMonitor;

private:
    bool _limpHome;
};

using SafeSupervisorConstructor = ::safety::explicitly_constructible<SafeSupervisor>;
extern SafeSupervisorConstructor safeSupervisorConstructor;

} // namespace safety
