// Copyright 2024 Accenture.

/**
 * Contains
 * \file
 * \ingroup
 */

#include "uds/DefaultEepromConstants.h"
#include "uds/DiagCodes.h"
#include "uds/DiagDispatcher.h"
#include "uds/DiagReturnCode.h"
#include "uds/DiagnosisConfiguration.h"
#include "uds/ICommunicationStateListener.h"
#include "uds/IDiagDispatcher.h"
#include "uds/UdsConfig.h"
#include "uds/UdsConstants.h"
#include "uds/application/AbstractDiagApplication.h"
#include "uds/async/AsyncDiagJob.h"
#include "uds/authentication/DefaultDiagAuthenticator.h"
#include "uds/authentication/IDiagAuthenticator.h"
#include "uds/base/AbstractDiagJob.h"
#include "uds/base/DiagJobRoot.h"
#include "uds/base/DiagJobWithAuthentication.h"
#include "uds/base/DiagJobWithAuthenticationAndSessionControl.h"
#include "uds/base/Service.h"
#include "uds/base/ServiceWithAuthentication.h"
#include "uds/base/ServiceWithAuthenticationAndSessionControl.h"
#include "uds/base/Subfunction.h"
#include "uds/base/SubfunctionWithAuthentication.h"
#include "uds/base/SubfunctionWithAuthenticationAndSessionControl.h"
#include "uds/connection/DiagConnectionManager.h"
#include "uds/connection/ErrorCode.h"
#include "uds/connection/IOutgoingDiagConnectionProvider.h"
#include "uds/connection/IncomingDiagConnection.h"
#include "uds/connection/ManagedOutgoingDiagConnection.h"
#include "uds/connection/OutgoingDiagConnection.h"
#include "uds/connection/PositiveResponse.h"
#include "uds/jobs/DataIdentifierJob.h"
#include "uds/jobs/RoutineControlJob.h"
#include "uds/services/communicationcontrol/CommunicationControl.h"
#include "uds/services/communicationcontrol/ICommunicationStateManager.h"
#include "uds/services/controldtcsetting/ControlDTCSetting.h"
#include "uds/services/ecureset/ECUReset.h"
#include "uds/services/ecureset/EnableRapidPowerShutdown.h"
#include "uds/services/ecureset/HardReset.h"
#include "uds/services/ecureset/PowerDown.h"
#include "uds/services/ecureset/SoftReset.h"
#include "uds/services/readdata/ReadDataByIdentifier.h"
#include "uds/services/routinecontrol/RequestRoutineResults.h"
#include "uds/services/routinecontrol/RoutineControl.h"
#include "uds/services/routinecontrol/StartRoutine.h"
#include "uds/services/routinecontrol/StopRoutine.h"
// #include "uds/services/securityaccess/SecurityAccess.h"
#include "uds/services/sessioncontrol/DiagnosticSessionControl.h"
#include "uds/services/sessioncontrol/EepromManagerSessionPersistence.h"
#include "uds/services/sessioncontrol/ISessionPersistence.h"
#include "uds/services/testerpresent/TesterPresent.h"
#include "uds/services/writedata/WriteDataByIdentifier.h"
#include "uds/session/ApplicationDefaultSession.h"
#include "uds/session/ApplicationExtendedSession.h"
#include "uds/session/IDiagSessionChangedListener.h"
#include "uds/session/IDiagSessionManager.h"
#include "uds/session/ProgrammingSession.h"

#include <gtest/gtest.h>

namespace
{
TEST(DiagnosisTest, IncludeCheck)
{
    // This test currently only checks if the includes are working.
}

} // anonymous namespace
