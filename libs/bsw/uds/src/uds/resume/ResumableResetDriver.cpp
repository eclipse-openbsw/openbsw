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
, fResumableDiagDispatcher(nullptr)
, fContext(context)
, fPersistence(persistence)
, fResumeMessage(resumeMessage)
, fResetTimeoutInMs(resetTimeoutInMs)
, fTesterObdEthSourceId(testerObdEthSourceId)
, fTesterObdEthStartupDelayInMs(testerObdEthStartupDelayInMs)
, fShutdownType(shutdownType)
{}

void ResumableResetDriver::resume(IResumableDiagDispatcher& resumableDiagDispatcher)
{
    fResumableDiagDispatcher = &resumableDiagDispatcher;
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
            (void)resumableDiagDispatcher.resume(fResumeMessage, nullptr);
        }
    }
}

void ResumableResetDriver::init(IResumableDiagDispatcher& resumableDiagDispatcher)
{
    fResumableDiagDispatcher = &resumableDiagDispatcher;
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
        fResumableDiagDispatcher->disable();
        return true;
    }
    else
    {
        return false;
    }
}

void ResumableResetDriver::abortReset()
{
    fPersistence.clear();
    fResumableDiagDispatcher->enable();
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

void ResumableResetDriver::lifecycleComplete() { resume(*fResumableDiagDispatcher); }

void ResumableResetDriver::execute()
{
    (void)fResumableDiagDispatcher->resume(fResumeMessage, nullptr);
}

} // namespace uds
