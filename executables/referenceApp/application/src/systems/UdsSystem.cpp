// Copyright 2024 Accenture.

#include "systems/UdsSystem.h"

#include "busid/BusId.h"
#include "lifecycle/LifecycleManager.h"
#include "transport/ITransportSystem.h"
#include "transport/TransportConfiguration.h"

#include <uds/UdsLogger.h>

#include <estd/type_traits.h>

namespace uds
{
using ::util::logger::Logger;
using ::util::logger::UDS;

uint8_t const responseData22Cf01[]
    = {0x01, 0x02, 0x00, 0x02, 0x22, 0x02, 0x16, 0x0F, 0x01, 0x00, 0x00, 0x6D,
       0x2F, 0x00, 0x00, 0x01, 0x06, 0x00, 0x00, 0x8F, 0xE0, 0x00, 0x00, 0x01};

UdsSystem::UdsSystem(
    lifecycle::LifecycleManager& lManager,
    transport::ITransportSystem& transportSystem,
    ::async::ContextType context,
    uint16_t udsAddress)
: AsyncLifecycleComponent()
, ::estd::singleton<UdsSystem>(*this)
, _udsLifecycleConnector(lManager)
, _transportSystem(transportSystem)
, _jobRoot()
, _diagnosticSessionControl(_udsLifecycleConnector, context, _dummySessionPersistence)
, _communicationControl()
, _udsConfiguration(
      udsAddress,
      transport::TransportConfiguration::FUNCTIONAL_ALL_ISO14229,
      ::busid::SELFDIAG,
      transport::TransportConfiguration::DIAG_PAYLOAD_SIZE,
      true,  /* activate outgoing and pending */
      false, /* accept all requests */
      true,  /* copy functional requests */
      context)
, _udsDispatcher(_udsConfiguration, _diagnosticSessionControl, _jobRoot, context)
, _readDataByIdentifier()
, _writeDataByIdentifier()
, _routineControl()
, _startRoutine()
, _stopRoutine()
, _requestRoutineResults()
, _read22Cf01(0xCF01, responseData22Cf01)
, _read22Cf02()
, _testerPresent()
, _context(context)
, _timeout()
{
    setTransitionContext(_context);
}

void UdsSystem::init()
{
    (void)_udsDispatcher.init();
    AbstractDiagJob::setDefaultDiagSessionManager(_diagnosticSessionControl);
    _diagnosticSessionControl.setDiagDispatcher(&_udsDispatcher);
    _transportSystem.addTransportLayer(_udsDispatcher);
    addDiagJobs();

    transitionDone();
}

void UdsSystem::run()
{
    ::async::scheduleAtFixedRate(_context, *this, _timeout, 10, ::async::TimeUnit::MILLISECONDS);
    transitionDone();
}

void UdsSystem::shutdown()
{
    removeDiagJobs();
    _diagnosticSessionControl.setDiagDispatcher(nullptr);
    _diagnosticSessionControl.shutdown();
    _transportSystem.removeTransportLayer(_udsDispatcher);
    (void)_udsDispatcher.shutdown(transport::AbstractTransportLayer::ShutdownDelegate::
                                      create<UdsSystem, &UdsSystem::shutdownComplete>(*this));
}

void UdsSystem::shutdownComplete(transport::AbstractTransportLayer&)
{
    _timeout.cancel();
    transitionDone();
}

DiagDispatcher2& UdsSystem::getUdsDispatcher() { return _udsDispatcher; }

IAsyncDiagHelper& UdsSystem::getAsyncDiagHelper() { return _asyncDiagHelper; }

IDiagSessionManager& UdsSystem::getDiagSessionManager() { return _diagnosticSessionControl; }

DiagnosticSessionControl& UdsSystem::getDiagnosticSessionControl()
{
    return _diagnosticSessionControl;
}

CommunicationControl& UdsSystem::getCommunicationControl() { return _communicationControl; }

ReadDataByIdentifier& UdsSystem::getReadDataByIdentifier() { return _readDataByIdentifier; }

void UdsSystem::addDiagJobs()
{
    // 22 - ReadDataByIdentifier
    (void)_udsDispatcher.addAbstractDiagJob(_readDataByIdentifier);
    (void)_udsDispatcher.addAbstractDiagJob(_read22Cf01);
    (void)_udsDispatcher.addAbstractDiagJob(_read22Cf02);

    // 2E - WriteDataByIdentifier
    (void)_udsDispatcher.addAbstractDiagJob(_writeDataByIdentifier);

    // 31 - Routine Control
    (void)_udsDispatcher.addAbstractDiagJob(_routineControl);
    (void)_udsDispatcher.addAbstractDiagJob(_startRoutine);
    (void)_udsDispatcher.addAbstractDiagJob(_stopRoutine);
    (void)_udsDispatcher.addAbstractDiagJob(_requestRoutineResults);

    // Services
    (void)_udsDispatcher.addAbstractDiagJob(_testerPresent);
    (void)_udsDispatcher.addAbstractDiagJob(_diagnosticSessionControl);
    (void)_udsDispatcher.addAbstractDiagJob(_communicationControl);
}

void UdsSystem::removeDiagJobs()
{
    // 22 - ReadDataByIdentifier
    (void)_udsDispatcher.removeAbstractDiagJob(_readDataByIdentifier);
    (void)_udsDispatcher.removeAbstractDiagJob(_read22Cf01);
    (void)_udsDispatcher.removeAbstractDiagJob(_read22Cf02);

    // 2E - WriteDataByIdentifier
    (void)_udsDispatcher.removeAbstractDiagJob(_writeDataByIdentifier);

    // 31 - Routine Control
    (void)_udsDispatcher.removeAbstractDiagJob(_routineControl);
    (void)_udsDispatcher.removeAbstractDiagJob(_startRoutine);
    (void)_udsDispatcher.removeAbstractDiagJob(_stopRoutine);
    (void)_udsDispatcher.removeAbstractDiagJob(_requestRoutineResults);

    // Services
    (void)_udsDispatcher.removeAbstractDiagJob(_testerPresent);
    (void)_udsDispatcher.removeAbstractDiagJob(_diagnosticSessionControl);
    (void)_udsDispatcher.removeAbstractDiagJob(_communicationControl);
}

void UdsSystem::execute() {}

} // namespace uds
