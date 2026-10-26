add_executable(
    doipTest
    src/doip/common/DoIpCyclicTaskGeneratorTest.cpp
    src/doip/common/DoIpHeaderTest.cpp
    src/doip/common/DoIpInterfaceCopyTest.cpp
    src/doip/common/DoIpResultTest.cpp
    src/doip/common/DoIpSendJobHelperTest.cpp
    src/doip/common/DoIpSimplePayloadSendJobTest.cpp
    src/doip/common/DoIpStaticPayloadSendJobTest.cpp
    src/doip/common/DoIpTcpConnectionTest.cpp
    src/doip/common/DoIpTestHelpers.cpp
    src/doip/common/DoIpTransportMessageProvidingAdapterTest.cpp
    src/doip/common/DoIpTransportMessageProvidingListenerHelperTest.cpp
    src/doip/common/DoIpTransportMessageRefTest.cpp
    src/doip/common/DoIpTransportMessageSendJobTest.cpp
    src/doip/common/DoIpUdpConnectionTest.cpp
    src/doip/common/DoIpVehicleIdentificationRequestSendJobTest.cpp
    src/doip/server/DoIpServerConnectionHandlerTest.cpp
    src/doip/server/DoIpServerSocketHandlerTest.cpp
    src/doip/server/DoIpServerTransportConnectionConfigTest.cpp
    src/doip/server/DoIpServerTransportConnectionPoolTest.cpp
    src/doip/server/DoIpServerTransportConnectionProviderTest.cpp
    src/doip/server/DoIpServerTransportConnectionTest.cpp
    src/doip/server/DoIpServerTransportLayerParametersTest.cpp
    src/doip/server/DoIpServerTransportLayerTest.cpp
    src/doip/server/DoIpServerTransportMessageHandlerTest.cpp
    src/doip/server/DoIpServerVehicleIdentificationConfigTest.cpp
    src/doip/server/DoIpServerVehicleIdentificationRequestTest.cpp
    src/doip/server/DoIpServerVehicleIdentificationServiceTest.cpp
    src/doip/server/DoIpServerVehicleIdentificationSocketHandlerTest.cpp
    src/doip/server/DoIpServerVehicleIdentificationTest.cpp
    src/doip/server/IDoIpServerConnectionFilterTest.cpp
    src/doip/server/IDoIpServerEntityStatusCallbackTest.cpp)

target_include_directories(doipTest PRIVATE include)

target_link_libraries(
    doipTest
    PRIVATE doip
            doipMock
            asyncMockImpl
            util
            utilMock
            cpp2ethernet
            cpp2ethernetMock
            transportMock
            bspMock
            gtest_main)

gtest_discover_tests(doipTest PROPERTIES LABELS "doipTest")
