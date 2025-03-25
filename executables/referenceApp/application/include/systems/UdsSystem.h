// Copyright 2024 Accenture.

#pragma once

#include <async/Async.h>
#include <async/IRunnable.h>
#include <lifecycle/AsyncLifecycleComponent.h>
#include <uds/DiagDispatcher.h>
#include <uds/DummySessionPersistence.h>
#include <uds/ReadIdentifierPot.h>
#include <uds/UdsLifecycleConnector.h>
#include <uds/async/AsyncDiagHelper.h>
#include <uds/async/AsyncDiagJob.h>
#include <uds/jobs/ReadIdentifierFromMemory.h>
#include <uds/services/communicationcontrol/CommunicationControl.h>
#include <uds/services/readdata/ReadDataByIdentifier.h>
#include <uds/services/routinecontrol/RequestRoutineResults.h>
#include <uds/services/routinecontrol/RoutineControl.h>
#include <uds/services/routinecontrol/StartRoutine.h>
#include <uds/services/routinecontrol/StopRoutine.h>
#include <uds/services/sessioncontrol/DiagnosticSessionControl.h>
#include <uds/services/testerpresent/TesterPresent.h>
#include <uds/services/writedata/WriteDataByIdentifier.h>

#include <estd/singleton.h>

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
, public ::estd::singleton<UdsSystem>
, private ::async::IRunnable
{
    using Base = ::estd::singleton<UdsSystem>;

public:
    UdsSystem(
        lifecycle::LifecycleManager& lManager,
        transport::ITransportSystem& transportSystem,
        ::async::ContextType context,
        uint16_t udsAddress);

    void init() override;
    void run() override;
    void shutdown() override;

    DiagDispatcher2& getUdsDispatcher();

    IAsyncDiagHelper& getAsyncDiagHelper();

    IDiagSessionManager& getDiagSessionManager();

    DiagnosticSessionControl& getDiagnosticSessionControl();

    CommunicationControl& getCommunicationControl();

    ReadDataByIdentifier& getReadDataByIdentifier();

private:
    void do_init(bool const wakingUp);

    void addDiagJobs();
    void removeDiagJobs();
    void shutdownTimeoutManager();
    void shutdownComplete(transport::AbstractTransportLayer&);
    void processDiagCluster();
    void execute() override;

    UdsLifecycleConnector _udsLifecycleConnector;

    transport::ITransportSystem& _transportSystem;
    DummySessionPersistence _dummySessionPersistence;

    DiagJobRoot _jobRoot;
    DiagnosticSessionControl _diagnosticSessionControl;
    CommunicationControl _communicationControl;
    DiagnosisConfiguration<5, 1, 16> _udsConfiguration;
    DiagDispatcher2 _udsDispatcher;
    uds::declare::AsyncDiagHelper<5> _asyncDiagHelper;

    ReadDataByIdentifier _readDataByIdentifier;
    WriteDataByIdentifier _writeDataByIdentifier;
    RoutineControl _routineControl;
    StartRoutine _startRoutine;
    StopRoutine _stopRoutine;
    RequestRoutineResults _requestRoutineResults;
    ReadIdentifierFromMemory _read22Cf01;
    ReadIdentifierPot _read22Cf02;
    TesterPresent _testerPresent;

    ::async::ContextType _context;
    ::async::TimeoutType _timeout;
};
} // namespace uds
