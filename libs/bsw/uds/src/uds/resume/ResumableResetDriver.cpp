// Copyright 2024 Accenture.

#include "uds/resume/ResumableResetDriver.h"

#include "uds/DiagDispatcher.h"
#include "uds/lifecycle/IUdsLifecycleConnector.h"
#include "uds/resume/IResumableResetDriverPersistence.h"

#include <async/Async.h>

namespace uds
{
ResumableResetDriver::ResumableResetDriver(
    IUdsLifecycleConnector& udsLifecycleConnector,
    ::async::ContextType context,
    IResumableResetDriverPersistence& persistence,
    ::transport::TransportMessage& resumeMessage,
    uint16_t const resetTimeoutInMs,
    IUdsLifecycleConnector::ShutdownType const shutdownType,
    uint16_t const testerObdEthSourceId,
    uint16_t const testerObdEthStartupDelayInMs)
: fUdsLifecycleConnector(udsLifecycleConnector)
, fDiagDispatcher(nullptr)
, fContext(context)
, fPersistence(persistence)
, fResumeMessage(resumeMessage)
, fResetTimeoutInMs(resetTimeoutInMs)
, fTesterObdEthSourceId(testerObdEthSourceId)
, fTesterObdEthStartupDelayInMs(testerObdEthStartupDelayInMs)
, fShutdownType(shutdownType)
{}

void ResumableResetDriver::resume(IDiagDispatcher& diagDispatcher)
{
    fDiagDispatcher = &diagDispatcher;
    if (fPersistence.readRequest(fResumeMessage))
    {
        fPersistence.clear();
        if (fResumeMessage.getPayloadLength() == 0U)
        {
            return;
        }
        if ((fResumeMessage.getSourceId() == fTesterObdEthSourceId)
            && (fTesterObdEthStartupDelayInMs > 0U))
        {
            ::async::schedule(
                fContext,
                *this,
                fTimeout,
                static_cast<uint32_t>(fTesterObdEthStartupDelayInMs),
                ::async::TimeUnit::MILLISECONDS);
        }
        else
        {
            (void)diagDispatcher.resume(fResumeMessage, nullptr);
        }
    }
}

void ResumableResetDriver::init(IDiagDispatcher& diagDispatcher)
{
    fDiagDispatcher = &diagDispatcher;
}

void ResumableResetDriver::shutdown() {}

bool ResumableResetDriver::prepareReset(::transport::TransportMessage const* const message)
{
    if (fUdsLifecycleConnector.isModeChangePossible())
    {
        if (message != nullptr)
        {
            fPersistence.writeRequest(*message);
        }
        else
        {
            fPersistence.clear();
        }
        fDiagDispatcher->fEnabled = false;
        return true;
    }

    return false;
}

void ResumableResetDriver::abortReset()
{
    fPersistence.clear();
    fDiagDispatcher->fEnabled = true;
}

void ResumableResetDriver::reset() { reset(fShutdownType, fResetTimeoutInMs); }

void ResumableResetDriver::reset(
    IUdsLifecycleConnector::ShutdownType const shutdownType, uint16_t const resetTimeoutInMs)
{
    if (!fUdsLifecycleConnector.requestShutdown(
            shutdownType, static_cast<uint32_t>(resetTimeoutInMs)))
    {
        abortReset();
    }
}

void ResumableResetDriver::lifecycleComplete() { resume(*fDiagDispatcher); }

void ResumableResetDriver::execute() { (void)fDiagDispatcher->resume(fResumeMessage, nullptr); }

} // namespace uds
