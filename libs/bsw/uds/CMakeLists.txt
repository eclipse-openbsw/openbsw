add_library(
    uds
    src/uds/async/AsyncDiagHelper.cpp
    src/uds/async/AsyncDiagJobHelper.cpp
    src/uds/authentication/DefaultDiagAuthenticator.cpp
    src/uds/base/AbstractDiagJob.cpp
    src/uds/base/DiagJobRoot.cpp
    src/uds/base/DiagJobWithAuthentication.cpp
    src/uds/base/DiagJobWithAuthenticationAndSessionControl.cpp
    src/uds/base/Service.cpp
    src/uds/base/ServiceWithAuthentication.cpp
    src/uds/base/ServiceWithAuthenticationAndSessionControl.cpp
    src/uds/base/Subfunction.cpp
    src/uds/base/SubfunctionWithAuthentication.cpp
    src/uds/base/SubfunctionWithAuthenticationAndSessionControl.cpp
    src/uds/connection/IncomingDiagConnection.cpp
    src/uds/connection/NestedDiagRequest.cpp
    src/uds/connection/PositiveResponse.cpp
    src/uds/jobs/DataIdentifierJob.cpp
    src/uds/jobs/ReadIdentifierFromMemory.cpp
    src/uds/jobs/ReadIdentifierFromMemoryWithAuthentication.cpp
    src/uds/jobs/ReadIdentifierFromSliceRef.cpp
    src/uds/jobs/RoutineControlJob.cpp
    src/uds/jobs/WriteIdentifierToMemory.cpp
    src/uds/resume/ResumableResetDriver.cpp
    src/uds/services/controldtcsetting/ControlDTCSetting.cpp
    src/uds/services/communicationcontrol/CommunicationControl.cpp
    src/uds/services/ecureset/ECUReset.cpp
    src/uds/services/ecureset/EnableRapidPowerShutdown.cpp
    src/uds/services/ecureset/HardReset.cpp
    src/uds/services/ecureset/PowerDown.cpp
    src/uds/services/ecureset/SoftReset.cpp
    src/uds/services/inputoutputcontrol/InputOutputControlByIdentifier.cpp
    src/uds/services/readdata/MultipleReadDataByIdentifier.cpp
    src/uds/services/readdata/ReadDataByIdentifier.cpp
    src/uds/services/routinecontrol/RequestRoutineResults.cpp
    src/uds/services/routinecontrol/RoutineControl.cpp
    src/uds/services/routinecontrol/StartRoutine.cpp
    src/uds/services/routinecontrol/StopRoutine.cpp
    src/uds/services/securityaccess/SecurityAccess.cpp
    src/uds/services/sessioncontrol/DiagnosticSessionControl.cpp
    src/uds/services/testerpresent/TesterPresent.cpp
    src/uds/services/writedata/WriteDataByIdentifier.cpp
    src/uds/DiagDispatcher.cpp
    src/uds/UdsLogger.cpp
    src/util/RoutineControlOptionParser.cpp)

target_include_directories(uds PUBLIC include)

target_link_libraries(
    uds
    PUBLIC async
           bsp
           configuration
           transport
           transportConfiguration
           udsConfiguration)

if (BUILD_EXECUTABLE STREQUAL "unitTest")

    add_subdirectory(mock)

    target_include_directories(uds PUBLIC test/mock/include)

    target_link_libraries(uds PUBLIC udsMock)

endif ()
