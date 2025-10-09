// Copyright 2024 Accenture.

#include "safeSupervisor/SafeSupervisor.h"

#include <etl/utility.h>
#include <interrupts/SuspendResumeAllInterruptsScopedLock.h>
#include <reset/softwareSystemReset.h>
#include <safeUtils/SafetyLogger.h>

#ifdef PLATFORM_SUPPORT_IO
#include <safeIo/SafeState.h>
#endif

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
, dmaEccMonitor(*this, Event::INTEGRITY_EVENT_DMA_ECC_ERROR)
, romCheckMonitor(*this, Event::ROM_CHECK_FAILURE)
, mpuEnableMonitor(*this, Event::MPU_NOT_ENABLED)
, mpuConfigMonitor(*this, Event::MPU_CONFIGURATION_ERROR)
, adcReferenceMonitor(*this, Event::ADC_REFERENCE_ERROR)
, internalSuppliesMonitor(*this, Event::INTERNAL_SUPPLIES_ERROR)
, checkRegisterMonitor(*this, Event::CHECK_REGISTER_ERROR)
, lockRegisterMonitor(*this, Event::LOCK_REGISTER_ERROR)
, _limpHome(true)
{}

void SafeSupervisor::init() const
{
    // TODO: Read and clear the no-init RAM
}

// Note: Currently the asynchronous logs here are broken due to the reset at the end of the
// function. This should be fixed after integration of no-init RAM, so tht the logs will be written
// after a restart.
// TODO: Make sure these logs are processed after a restart once no-init RAM is available
void SafeSupervisor::handle(Event const& event)
{
    ::interrupts::SuspendResumeAllInterruptsScopedLock const lock;
    switch (event)
    {
        case Event::SAFETY_MANAGER_SEQUENCE_DEVIATION:
        {
            Logger::error(SAFETY, "Event: SAFETY_MANAGER_SEQUENCE_DEVIATION");
            // TODO: remove the log, and replace it with writing the event to no-init ram
            // Note: use event.getContext to get extra information about the event
            break;
        }
        case Event::WATCHDOG_STARTUP_CHECK_FAILURE:
        {
            Logger::error(SAFETY, "Event: WATCHDOG_STARTUP_CHECK_FAILURE");
            return;
        }
        case Event::SAFE_WATCHDOG_SEQUENCE_DEVIATION:
        {
            Logger::error(SAFETY, "Event: SAFE_WATCHDOG_SEQUENCE_DEVIATION");
            // TODO: remove the log, and replace it with writing the event to no-init ram
            // Note: use event.getContext to get extra information about the event
            break;
        }
        case Event::SAFE_WATCHDOG_CONFIGURATION_ERROR:
        {
            Logger::error(SAFETY, "Event: SAFE_WATCHDOG_CONFIGURATION_ERROR");
            // TODO: remove the log, and replace it with writing the event to no-init ram
            // Note: use event.getContext to get extra information about the event
            break;
        }
        case Event::SAFE_WATCHDOG_SERVICE_DEVIATION:
        {
            Logger::error(SAFETY, "Event: SAFE_WATCHDOG_SERVICE_DEVIATION");
            // TODO: remove the log, and replace it with writing the event to no-init ram
            // Note: use event.getContext to get extra information about the event
            break;
        }
        case Event::MPU_UNLOCKED_ON_SAFETY_MANAGER_ENTRY:
        {
            Logger::error(SAFETY, "Event: MPU_UNLOCKED_ON_SAFETY_MANAGER_ENTRY");
            // TODO: remove the log, and replace it with writing the event to no-init ram
            // Note: use event.getContext to get extra information about the event
            break;
        }
        case Event::MPU_LOCKED_ON_SAFETY_MANAGER_EXIT:
        {
            Logger::error(SAFETY, "Event: MPU_LOCKED_ON_SAFETY_MANAGER_EXIT");
            // TODO: remove the log, and replace it with writing the event to no-init ram
            // Note: use event.getContext to get extra information about the event
            break;
        }
        case Event::INTEGRITY_EVENT_DMA_ECC_ERROR:
        {
            Logger::error(SAFETY, "Event: INTEGRITY_EVENT_DMA_ECC_ERROR, System Reset");
            // TODO: remove the log, and replace it with writing the event to no-init ram
            // Note: use event.getContext to get extra information about the event
            break;
        }
        case Event::ROM_CHECK_FAILURE:
        {
            Logger::error(SAFETY, "Event: ROM_CHECK_FAILURE");
            // TODO: remove the log, and replace it with writing the event to no-init ram
            // Note: use event.getContext to get extra information about the event
            return;
        }
        case Event::MPU_NOT_ENABLED:
        {
            Logger::error(SAFETY, "Event: MPU_NOT_ENABLED");
            // TODO: remove the log, and replace it with writing the event to no-init ram
            // Note: use event.getContext to get extra information about the event
            break;
        }
        case Event::MPU_CONFIGURATION_ERROR:
        {
            uint32_t const region = mpuConfigMonitor.getContext();
            Logger::error(SAFETY, "Event: MPU_CONFIGURATION_ERROR, failed region: %d", region);
            // TODO: remove the log, and replace it with writing the event to no-init ram
            // Note: use event.getContext to get extra information about the event
            break;
        }
        case Event::ADC_REFERENCE_ERROR:
        {
            Logger::error(SAFETY, "Event: ADC_REFERENCE_ERROR");
            // TODO: remove the log, and replace it with writing the event to no-init ram
            // Note: use event.getContext to get extra information about the event
            break;
        }
        case Event::INTERNAL_SUPPLIES_ERROR:
        {
            Logger::error(SAFETY, "Event: INTERNAL_SUPPLIES_ERROR");
            // TODO: remove the log, and replace it with writing the event to no-init ram
            // Note: use event.getContext to get extra information about the event
            break;
        }
        case Event::CHECK_REGISTER_ERROR:
        {
#ifdef PLATFORM_SUPPORT_IO
            bool const ok = checkRegisterMonitor.getContext();
            if (ok)
            {
                Logger::error(SAFETY, "Event: CHECK_REGISTER_ERROR (leave safe state)");
                ::safety::SafeState::leaveSafeState();
            }
            else
            {
                Logger::error(SAFETY, "Event: CHECK_REGISTER_ERROR (enter safe state)");
                ::safety::SafeState::enterSafeState();
            }
#else
            Logger::error(SAFETY, "Event: CHECK_REGISTER_ERROR");
#endif
            // This is not a fatal error, so we do not enter limp home or reset the MCU.
            return;
        }
        case Event::LOCK_REGISTER_ERROR:
        {
            auto const reg_id = lockRegisterMonitor.getContext();
            Logger::error(SAFETY, "Event: LOCK_REGISTER_ERROR, failed register id: %d", reg_id);
            // TODO: remove the log, and replace it with writing the event to no-init ram
            break;
        }
        default:
        {
            Logger::warn(
                SAFETY,
                "SafeSupervisor: Received unknown event with ID %d",
                ::etl::to_underlying(event));
            // TODO: write event id (int) to no-init ram as unknown error
            break;
        }
    }
    enterLimpHome();
    // reset the MCU
    softwareSystemReset();
}

SafeSupervisorConstructor safeSupervisorConstructor;

SafeSupervisor& SafeSupervisor::getInstance() { return *safeSupervisorConstructor; }

} // namespace safety
