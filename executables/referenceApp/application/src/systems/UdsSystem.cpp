// Copyright 2024 Accenture.

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

// VIN (DID F190) — 17-byte writable scratchpad, matching Rust firmware
etl::array<uint8_t, 17> vinData
    = {'O', 'B', 'S', 'W', 'S', 'T', 'M', '3', '2', '0', '0', '0', '0', '0', '0', '0', '1'};

// SW version (DID F195) — read-only
uint8_t const swVersionData[] = {'1', '.', '0', '.', '0', '-', 's', 't', 'm', '3', '2'};

// ECU serial number (DID F18C)
uint8_t const ecuSerialData[] = {'O', 'B', 'S', 'W', '-', 'S', 'T', 'M', '3', '2', '-', '0', '0', '1'};

// HW version (DID F193)
uint8_t const hwVersionData[] = {'H', 'W', '-', '1', '.', '0'};

// Supplier ID (DID F18A)
uint8_t const supplierIdData[] = {'E', 'c', 'l', 'i', 'p', 's', 'e', '-', 'O', 'B', 'S', 'W'};

// Boot SW version (DID F180)
uint8_t const bootSwVersionData[] = {'B', 'O', 'O', 'T', '-', '1', '.', '0'};


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
      true,  /* accept all requests */
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
, _readF190(0xF190, ::etl::span<uint8_t const>(vinData.data(), vinData.size()))
, _writeF190(0xF190, ::etl::span<uint8_t>(vinData.data(), vinData.size()))
, _readF195(0xF195, swVersionData, sizeof(swVersionData))
, _readF18C(0xF18C, ecuSerialData, sizeof(ecuSerialData))
, _readF193(0xF193, hwVersionData, sizeof(hwVersionData))
, _readF18A(0xF18A, supplierIdData, sizeof(supplierIdData))
, _readF180(0xF180, bootSwVersionData, sizeof(bootSwVersionData))
, _testerPresent()
, _ecuReset()
, _hardReset(_udsLifecycleConnector, _udsDispatcher)
, _securityAccess()
, _dtcManager()
, _clearDtc(_dtcManager)
, _readDtcInfo(_dtcManager)
, _controlDtcSetting(_dtcManager)
, _routineFF00(0xFF00U)
, _routineFF01(0xFF01U)
, _routineFF02(0xFF02U)
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
    // 10 - DiagnosticSessionControl
    (void)_jobRoot.addAbstractDiagJob(_diagnosticSessionControl);

    // 11 - ECUReset
    (void)_jobRoot.addAbstractDiagJob(_ecuReset);
    (void)_jobRoot.addAbstractDiagJob(_hardReset);

    // 14 - ClearDiagnosticInformation
    (void)_jobRoot.addAbstractDiagJob(_clearDtc);

    // 19 - ReadDTCInformation
    (void)_jobRoot.addAbstractDiagJob(_readDtcInfo);

    // 22 - ReadDataByIdentifier
    (void)_jobRoot.addAbstractDiagJob(_readDataByIdentifier);
    (void)_jobRoot.addAbstractDiagJob(_read22Cf01);
    (void)_jobRoot.addAbstractDiagJob(_read22Cf02);
    (void)_jobRoot.addAbstractDiagJob(_readF190);
    (void)_jobRoot.addAbstractDiagJob(_readF195);
    (void)_jobRoot.addAbstractDiagJob(_readF18C);
    (void)_jobRoot.addAbstractDiagJob(_readF193);
    (void)_jobRoot.addAbstractDiagJob(_readF18A);
    (void)_jobRoot.addAbstractDiagJob(_readF180);

    // 27 - SecurityAccess
    (void)_jobRoot.addAbstractDiagJob(_securityAccess);

    // 28 - CommunicationControl
    (void)_jobRoot.addAbstractDiagJob(_communicationControl);

    // 2E - WriteDataByIdentifier
    (void)_jobRoot.addAbstractDiagJob(_writeDataByIdentifier);
    (void)_jobRoot.addAbstractDiagJob(_write2eCf03);
    (void)_jobRoot.addAbstractDiagJob(_writeF190);

    // 31 - Routine Control
    (void)_jobRoot.addAbstractDiagJob(_routineControl);
    (void)_jobRoot.addAbstractDiagJob(_startRoutine);
    (void)_jobRoot.addAbstractDiagJob(_stopRoutine);
    (void)_jobRoot.addAbstractDiagJob(_requestRoutineResults);
    (void)_jobRoot.addAbstractDiagJob(_routineFF00.getStartRoutine());
    (void)_jobRoot.addAbstractDiagJob(_routineFF00.getStopRoutine());
    (void)_jobRoot.addAbstractDiagJob(_routineFF00.getRequestRoutineResults());
    (void)_jobRoot.addAbstractDiagJob(_routineFF01.getStartRoutine());
    (void)_jobRoot.addAbstractDiagJob(_routineFF01.getStopRoutine());
    (void)_jobRoot.addAbstractDiagJob(_routineFF01.getRequestRoutineResults());
    (void)_jobRoot.addAbstractDiagJob(_routineFF02.getStartRoutine());
    (void)_jobRoot.addAbstractDiagJob(_routineFF02.getStopRoutine());
    (void)_jobRoot.addAbstractDiagJob(_routineFF02.getRequestRoutineResults());

    // 3E - TesterPresent
    (void)_jobRoot.addAbstractDiagJob(_testerPresent);

    // 85 - ControlDTCSetting
    (void)_jobRoot.addAbstractDiagJob(_controlDtcSetting);
}

void UdsSystem::removeDiagJobs()
{
    _jobRoot.removeAbstractDiagJob(_diagnosticSessionControl);
    _jobRoot.removeAbstractDiagJob(_ecuReset);
    _jobRoot.removeAbstractDiagJob(_hardReset);
    _jobRoot.removeAbstractDiagJob(_clearDtc);
    _jobRoot.removeAbstractDiagJob(_readDtcInfo);
    _jobRoot.removeAbstractDiagJob(_readDataByIdentifier);
    _jobRoot.removeAbstractDiagJob(_read22Cf01);
    _jobRoot.removeAbstractDiagJob(_read22Cf02);
    _jobRoot.removeAbstractDiagJob(_readF190);
    _jobRoot.removeAbstractDiagJob(_readF195);
    _jobRoot.removeAbstractDiagJob(_readF18C);
    _jobRoot.removeAbstractDiagJob(_readF193);
    _jobRoot.removeAbstractDiagJob(_readF18A);
    _jobRoot.removeAbstractDiagJob(_readF180);
    _jobRoot.removeAbstractDiagJob(_securityAccess);
    _jobRoot.removeAbstractDiagJob(_communicationControl);
    _jobRoot.removeAbstractDiagJob(_writeDataByIdentifier);
    _jobRoot.removeAbstractDiagJob(_write2eCf03);
    _jobRoot.removeAbstractDiagJob(_writeF190);
    _jobRoot.removeAbstractDiagJob(_routineControl);
    _jobRoot.removeAbstractDiagJob(_startRoutine);
    _jobRoot.removeAbstractDiagJob(_stopRoutine);
    _jobRoot.removeAbstractDiagJob(_requestRoutineResults);
    _jobRoot.removeAbstractDiagJob(_routineFF00.getStartRoutine());
    _jobRoot.removeAbstractDiagJob(_routineFF00.getStopRoutine());
    _jobRoot.removeAbstractDiagJob(_routineFF00.getRequestRoutineResults());
    _jobRoot.removeAbstractDiagJob(_routineFF01.getStartRoutine());
    _jobRoot.removeAbstractDiagJob(_routineFF01.getStopRoutine());
    _jobRoot.removeAbstractDiagJob(_routineFF01.getRequestRoutineResults());
    _jobRoot.removeAbstractDiagJob(_routineFF02.getStartRoutine());
    _jobRoot.removeAbstractDiagJob(_routineFF02.getStopRoutine());
    _jobRoot.removeAbstractDiagJob(_routineFF02.getRequestRoutineResults());
    _jobRoot.removeAbstractDiagJob(_testerPresent);
    _jobRoot.removeAbstractDiagJob(_controlDtcSetting);
}

void UdsSystem::execute() {}

} // namespace uds
