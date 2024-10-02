add_executable(
    udsTest
    src/uds/async/AsyncDiagHelperTest.cpp
    src/uds/async/AsyncDiagJobHelperTest.cpp
    src/uds/async/AsyncDiagJobTest.cpp
    src/uds/authentication/DefaultDiagAuthenticatorTest.cpp
    src/uds/base/AbstractDiagJobTest.cpp
    src/uds/base/AbstractDiagJobWithDiagRoot.cpp
    src/uds/base/DiagJobRootTest.cpp
    src/uds/base/DiagJobWithAuthenticationAndSessionControlTest.cpp
    src/uds/base/DiagJobWithAuthenticationTest.cpp
    src/uds/base/ServiceTest.cpp
    src/uds/base/ServiceWithAuthenticationAndSessionControlTest.cpp
    src/uds/base/ServiceWithAuthenticationTest.cpp
    src/uds/base/SubfunctionTest.cpp
    src/uds/base/SubfunctionWithAuthenticationAndSessionControlTest.cpp
    src/uds/base/SubfunctionWithAuthenticationTest.cpp
    src/uds/connection/ManagedIncomingDiagConnectionTest.cpp
    src/uds/connection/NestedDiagRequestTest.cpp
    src/uds/connection/PositiveResponseTest.cpp
    src/uds/jobs/DataIdentifierJobTest.cpp
    src/uds/jobs/ReadIdentifierFromMemoryJobTest.cpp
    src/uds/jobs/ReadIdentifierFromMemoryWithAuthenticationTest.cpp
    src/uds/jobs/ReadIdentifierFromSliceRefTest.cpp
    src/uds/jobs/RoutineControlJobTest.cpp
    src/uds/jobs/WritedentifierToMemoryJobTest.cpp
    src/uds/resume/ResumableResetDriverTest.cpp
    src/uds/services/controldtcsetting/ControlDTCSettingTest.cpp
    src/uds/services/readdata/MultipleReadDataByIdentifierTest.cpp
    src/uds/services/readdata/ReadDataByIdentifierTest.cpp
    src/uds/services/routinecontrol/RequestRoutineResultsTest.cpp
    src/uds/services/routinecontrol/RoutineControlTest.cpp
    src/uds/services/routinecontrol/StartRoutineTest.cpp
    src/uds/services/routinecontrol/StopRoutineTest.cpp
    src/uds/services/securityaccess/SecurityAccessTest.cpp
    src/uds/services/sessioncontrol/DiagnosticSessionControlTest.cpp
    src/uds/services/testerpresent/TesterPresentTest.cpp
    src/uds/services/writedata/WriteDataByIdentifierTest.cpp
    src/uds/services/CommunicationControlTest.cpp
    src/uds/IncludeTest.cpp
    src/uds/IntegrationTest.cpp
    src/util/RoutineControlOptionParserTest.cpp
    ../src/transport/TransportMessageWithBuffer.cpp
    ../src/uds/session/DiagSession.cpp
    ../src/Logger.cpp)

target_include_directories(udsTest PRIVATE include ../include)

target_link_libraries(
    udsTest
    PRIVATE uds
            commonTimeout
            udsMock
            transportMock
            utCommon
            estdMock
            utilMock
            asyncMockImpl
            gtest_main)

gtest_discover_tests(udsTest PROPERTIES LABELS "udsTest")
