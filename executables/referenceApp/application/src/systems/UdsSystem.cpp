/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "systems/UdsSystem.h"

#include <busid/BusId.h>
#include <etl/array.h>
#include <lifecycle/LifecycleManager.h>
#include <transport/ITransportSystem.h>
#include <transport/TransportConfiguration.h>

namespace uds
{
uint8_t const responseData22Cf01[]
    = {0x01, 0x02, 0x00, 0x02, 0x22, 0x02, 0x16, 0x0F, 0x01, 0x00, 0x00, 0x6D,
       0x2F, 0x00, 0x00, 0x01, 0x06, 0x00, 0x00, 0x8F, 0xE0, 0x00, 0x00, 0x01};

etl::array<uint8_t, 3> storedData2eCf03 = {0};

UdsSystem::UdsSystem(
    lifecycle::LifecycleManager& lManager,
    transport::ITransportSystem& transportSystem,
    ::async::ContextType context,
    uint16_t udsAddress)
: AsyncLifecycleComponent()
, ::etl::singleton_base<UdsSystem>(*this)
, _udsLifecycleConnector(lManager)
, _transportSystem(transportSystem)
, _jobRoot()
, _diagnosticSessionControl(_udsLifecycleConnector, context, _dummySessionPersistence)
, _communicationControl()
, _udsConfiguration{
      udsAddress,
      transport::TransportConfiguration::FUNCTIONAL_ALL_ISO14229,
      transport::TransportConfiguration::DIAG_PAYLOAD_SIZE,
      ::busid::SELFDIAG,
      true,  /* activate outgoing pending */
      false, /* accept all requests */
      true,  /* copy functional requests */
      context}
, _udsDispatcher(
      _connectionPool, _sendJobQueue, _udsConfiguration, _diagnosticSessionControl, _jobRoot)
, _asyncDiagHelper(context)
, _readDataByIdentifier()
, _writeDataByIdentifier()
, _routineControl()
, _startRoutine()
, _stopRoutine()
, _requestRoutineResults()
, _read22Cf01(0xCF01, responseData22Cf01)
, _read22Cf02()
, _write2eCf03(0xCF03, storedData2eCf03)
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

DiagDispatcher& UdsSystem::getUdsDispatcher() { return _udsDispatcher; }

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
    // 14 - ClearDiagnosticInformation
    (void)_jobRoot.addAbstractDiagJob(_clearDiagnosticInformation);

    // 22 - ReadDataByIdentifier
    (void)_jobRoot.addAbstractDiagJob(_readDataByIdentifier);
    (void)_jobRoot.addAbstractDiagJob(_read22Cf01);
    (void)_jobRoot.addAbstractDiagJob(_read22Cf02);

    // 2E - WriteDataByIdentifier
    (void)_jobRoot.addAbstractDiagJob(_writeDataByIdentifier);
    (void)_jobRoot.addAbstractDiagJob(_write2eCf03);

    // 31 - Routine Control
    (void)_jobRoot.addAbstractDiagJob(_routineControl);
    (void)_jobRoot.addAbstractDiagJob(_startRoutine);
    (void)_jobRoot.addAbstractDiagJob(_stopRoutine);
    (void)_jobRoot.addAbstractDiagJob(_requestRoutineResults);

    // Services
    (void)_jobRoot.addAbstractDiagJob(_testerPresent);
    (void)_jobRoot.addAbstractDiagJob(_diagnosticSessionControl);
    (void)_jobRoot.addAbstractDiagJob(_communicationControl);
    (void)_jobRoot.addAbstractDiagJob(_readDTCInformation);
}

void UdsSystem::removeDiagJobs()
{
    // 14 - ClearDiagnosticInformation
    _jobRoot.removeAbstractDiagJob(_clearDiagnosticInformation);

    // 22 - ReadDataByIdentifier
    _jobRoot.removeAbstractDiagJob(_readDataByIdentifier);
    _jobRoot.removeAbstractDiagJob(_read22Cf01);
    _jobRoot.removeAbstractDiagJob(_read22Cf02);

    // 2E - WriteDataByIdentifier
    _jobRoot.removeAbstractDiagJob(_writeDataByIdentifier);
    _jobRoot.removeAbstractDiagJob(_write2eCf03);

    // 31 - Routine Control
    _jobRoot.removeAbstractDiagJob(_routineControl);
    _jobRoot.removeAbstractDiagJob(_startRoutine);
    _jobRoot.removeAbstractDiagJob(_stopRoutine);
    _jobRoot.removeAbstractDiagJob(_requestRoutineResults);

    // Services
    _jobRoot.removeAbstractDiagJob(_testerPresent);
    _jobRoot.removeAbstractDiagJob(_diagnosticSessionControl);
    _jobRoot.removeAbstractDiagJob(_communicationControl);
    _jobRoot.removeAbstractDiagJob(_readDTCInformation);
}

void UdsSystem::execute() {}

} // namespace uds
