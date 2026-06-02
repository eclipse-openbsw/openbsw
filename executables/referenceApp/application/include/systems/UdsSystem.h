/********************************************************************************
 * Copyright (c) 2024 Accenture
 * Copyright (c) 2026 An Dao
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <async/Async.h>
#include <async/IRunnable.h>
#include <etl/singleton_base.h>
#include <lifecycle/AsyncLifecycleComponent.h>
#include <uds/DiagDispatcher.h>
#include <uds/DummySessionPersistence.h>
#include <uds/ReadIdentifierPot.h>
#include <uds/UdsLifecycleConnector.h>
#include <uds/async/AsyncDiagHelper.h>
#include <uds/async/AsyncDiagJob.h>
#include <uds/jobs/ReadIdentifierFromMemory.h>
#include <uds/jobs/WriteIdentifierToMemory.h>

#ifdef PLATFORM_SUPPORT_UDS_DEMO_SERVICES
#include <uds/Stm32ClearDtc.h>
#include <uds/Stm32ControlDtcSetting.h>
#include <uds/Stm32DemoRoutine.h>
#include <uds/Stm32DtcManager.h>
#include <uds/Stm32ReadDtcInfo.h>
#include <uds/Stm32SecurityAccess.h>
#include <uds/Stm32WriteVin.h>
#include <uds/services/ecureset/ECUReset.h>
#include <uds/services/ecureset/HardReset.h>
#endif

#include <uds/services/communicationcontrol/CommunicationControl.h>
#include <uds/services/readdata/ReadDataByIdentifier.h>
#include <uds/services/routinecontrol/RequestRoutineResults.h>
#include <uds/services/routinecontrol/RoutineControl.h>
#include <uds/services/routinecontrol/StartRoutine.h>
#include <uds/services/routinecontrol/StopRoutine.h>
#include <uds/services/sessioncontrol/DiagnosticSessionControl.h>
#include <uds/services/testerpresent/TesterPresent.h>
#include <uds/services/writedata/WriteDataByIdentifier.h>

namespace lifecycle
{
class LifecycleManager;
}

namespace transport
{
class ITransportSystem;
}

namespace uds
{
class UdsSystem
: public lifecycle::AsyncLifecycleComponent
, public ::etl::singleton_base<UdsSystem>
, private ::async::IRunnable
{
public:
    UdsSystem(
        lifecycle::LifecycleManager& lManager,
        transport::ITransportSystem& transportSystem,
        ::async::ContextType context,
        uint16_t udsAddress);

    void init() override;
    void run() override;
    void shutdown() override;

    DiagDispatcher& getUdsDispatcher();

    IAsyncDiagHelper& getAsyncDiagHelper();

    IDiagSessionManager& getDiagSessionManager();

    DiagnosticSessionControl& getDiagnosticSessionControl();

    CommunicationControl& getCommunicationControl();

    ReadDataByIdentifier& getReadDataByIdentifier();

private:
    void addDiagJobs();
    void removeDiagJobs();
    void shutdownComplete(transport::AbstractTransportLayer&);
    void execute() override;

    UdsLifecycleConnector _udsLifecycleConnector;

    transport::ITransportSystem& _transportSystem;
    DummySessionPersistence _dummySessionPersistence;

    DiagJobRoot _jobRoot;
    DiagnosticSessionControl _diagnosticSessionControl;
    CommunicationControl _communicationControl;
    DiagnosisConfiguration _udsConfiguration;
    ::etl::pool<IncomingDiagConnection, 5> _connectionPool;
    ::etl::queue<TransportJob, 16> _sendJobQueue;
    DiagDispatcher _udsDispatcher;
    uds::declare::AsyncDiagHelper<5> _asyncDiagHelper;

    ReadDataByIdentifier _readDataByIdentifier;
    WriteDataByIdentifier _writeDataByIdentifier;
    RoutineControl _routineControl;
    StartRoutine _startRoutine;
    StopRoutine _stopRoutine;
    RequestRoutineResults _requestRoutineResults;
    ReadIdentifierFromMemory _read22Cf01;
    ReadIdentifierPot _read22Cf02;
    WriteIdentifierToMemory _write2eCf03;
#ifdef PLATFORM_SUPPORT_UDS_DEMO_SERVICES
    ReadIdentifierFromMemory _readF190;
    Stm32WriteVin _writeF190;
    ReadIdentifierFromMemory _readF195;
    ReadIdentifierFromMemory _readF18C;
    ReadIdentifierFromMemory _readF193;
    ReadIdentifierFromMemory _readF18A;
    ReadIdentifierFromMemory _readF180;
#endif
    TesterPresent _testerPresent;
#ifdef PLATFORM_SUPPORT_UDS_DEMO_SERVICES
    ECUReset _ecuReset;
    HardReset _hardReset;
    Stm32SecurityAccess _securityAccess;
    Stm32DtcManager _dtcManager;
    Stm32ClearDtc _clearDtc;
    Stm32ReadDtcInfo _readDtcInfo;
    Stm32ControlDtcSetting _controlDtcSetting;
    Stm32DemoRoutine _routineFF00;
    Stm32DemoRoutine _routineFF01;
    Stm32DemoRoutine _routineFF02;
#endif

    ::async::ContextType _context;
    ::async::TimeoutType _timeout;
};
} // namespace uds
