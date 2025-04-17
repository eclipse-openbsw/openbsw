// Copyright 2024 Accenture.

#include "safeLifecycle/SafeSupervisor.h"

#include "safeLifecycle/SafetyLogger.h"

#include <safeLifecycle/interrupts/IsrLock.h>

#include <estd/type_utils.h>

#include <commonDebug.h>

namespace safety
{

using ::util::logger::Logger;
using ::util::logger::SAFETY;

SafeSupervisor::SafeSupervisor()
: safetyManagerSequenceMonitor(
    *this,
    Event::SAFETY_MANAGER_SEQUENCE_DEVIATION,
    SafetyManagerSequence::SAFETY_MANAGER_ENTER,
    SafetyManagerSequence::SAFETY_MANAGER_LEAVE)
, watchdogStartupCheckMonitor(*this, Event::WATCHDOG_STARTUP_CHECK_FAILURE)
, safeWatchdogSequenceMonitor(
      *this,
      Event::SAFE_WATCHDOG_SEQUENCE_DEVIATION,
      EnterLeaveSequence::ENTER,
      EnterLeaveSequence::LEAVE)
, safeWatchdogConfigMonitor(*this, Event::SAFE_WATCHDOG_CONFIGURATION_ERROR, true)
, serviceWatchdogMonitor(*this, Event::SAFE_WATCHDOG_SERVICE_DEVIATION)
, mpuStatusCheckOnEnterMonitor(*this, Event::MPU_UNLOCKED_ON_SAFETY_MANAGER_ENTRY, true)
, mpuStatusCheckOnLeaveMonitor(*this, Event::MPU_LOCKED_ON_SAFETY_MANAGER_EXIT, false)
, _limpHome(true)
{}

void SafeSupervisor::init() const
{
    // TODO: read the no-init ram and clear no init ram?
}

void SafeSupervisor::handle(Event const& event)
{
    safety::IsrLock const lock;
    switch (event)
    {
        case Event::SAFE_EVENT_DUMMY:
        {
            Logger::debug(SAFETY, "Event: SAFE_EVENT_DUMMY");
            // TODO: remove the log, and replace it with writing the event to no-init ram
            // Note: use event.getContext to get extra information about the event
            break;
        }
        case Event::SAFETY_MANAGER_SEQUENCE_DEVIATION:
        {
            Logger::debug(SAFETY, "Event: SAFETY_MANAGER_SEQUENCE_DEVIATION");
            // TODO: remove the log, and replace it with writing the event to no-init ram
            // Note: use event.getContext to get extra information about the event
            break;
        }
        case Event::WATCHDOG_STARTUP_CHECK_FAILURE:
        {
            LOGSYNCHRON("Event: WATCHDOG_STARTUP_CHECK_FAILURE\n");
            break;
        }
        case Event::SAFE_WATCHDOG_SEQUENCE_DEVIATION:
        {
            Logger::debug(SAFETY, "Event: SAFE_WATCHDOG_SEQUENCE_DEVIATION");
            // TODO: remove the log, and replace it with writing the event to no-init ram
            // Note: use event.getContext to get extra information about the event
            break;
        }
        case Event::SAFE_WATCHDOG_CONFIGURATION_ERROR:
        {
            Logger::debug(SAFETY, "Event: SAFE_WATCHDOG_CONFIGURATION_ERROR");
            // TODO: remove the log, and replace it with writing the event to no-init ram
            // Note: use event.getContext to get extra information about the event
            break;
        }
        case Event::SAFE_WATCHDOG_SERVICE_DEVIATION:
        {
            Logger::debug(SAFETY, "Event: SAFE_WATCHDOG_SERVICE_DEVIATION");
            break;
        }
        case Event::MPU_UNLOCKED_ON_SAFETY_MANAGER_ENTRY:
        {
            Logger::debug(SAFETY, "Event: MPU_UNLOCKED_ON_SAFETY_MANAGER_ENTRY");
            // TODO: remove the log, and replace it with writing the event to no-init ram
            // Note: use event.getContext to get extra information about the event
            break;
        }
        case Event::MPU_LOCKED_ON_SAFETY_MANAGER_EXIT:
        {
            Logger::debug(SAFETY, "Event: MPU_LOCKED_ON_SAFETY_MANAGER_EXIT");
            // TODO: remove the log, and replace it with writing the event to no-init ram
            // Note: use event.getContext to get extra information about the event
            break;
        }
        default:
        {
            Logger::warn(
                SAFETY,
                "SafeSupervisor: Received unknown event with ID %d",
                static_cast<int>(::estd::to_underlying(event)));
            // TODO: write event id (int) to no-init ram as unknown error
            break;
        }
    }
    enterLimpHome();
    // reset the MCU
}

SafeSupervisor& SafeSupervisor::getInstance()
{
    // TODO: the getInstance function needs to be changed.
    // no function static variables are allowed within safety code
    // the construction of the SafeSupervisor instance needs to be deferred until after the
    // activation of the MCU
    static SafeSupervisor instance;
    return instance;
}

void SafeSupervisor::enterLimpHome() { _limpHome = true; }

void SafeSupervisor::leaveLimpHome() { _limpHome = false; }

bool SafeSupervisor::limpHome() const { return _limpHome; }

} // namespace safety
