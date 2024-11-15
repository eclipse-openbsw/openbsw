add_executable(
    docanTest
    src/docan/addressing/DoCanNormalAddressingFilterTest.cpp
    src/docan/addressing/DoCanNormalAddressingTest.cpp
    src/docan/can/DoCanPhysicalCanTransceiverContainerTest.cpp
    src/docan/can/DoCanPhysicalCanTransceiverTest.cpp
    src/docan/common/DoCanConnectionTest.cpp
    src/docan/common/DoCanParametersTest.cpp
    src/docan/common/DoCanTransportAddressPairTest.cpp
    src/docan/datalink/DoCanDataLinkAddressPairTest.cpp
    src/docan/datalink/DoCanFrameCodecTest.cpp
    src/docan/datalink/DoCanFrameDecoderTest.cpp
    src/docan/receiver/DoCanMessageReceiveProtocolHandlerTest.cpp
    src/docan/receiver/DoCanMessageReceiverTest.cpp
    src/docan/receiver/DoCanReceiverTest.cpp
    src/docan/transmitter/DoCanMessageTransmitProtocolHandlerTest.cpp
    src/docan/transmitter/DoCanMessageTransmitterTest.cpp
    src/docan/transmitter/DoCanTransmitterTest.cpp
    src/docan/transport/DoCanTransportLayerConfigTest.cpp
    src/docan/transport/DoCanTransportLayerContainerTest.cpp
    src/docan/transport/DoCanTransportLayerTest.cpp
    src/docan/DoCanIncludeTest.cpp
    src/docan/integration/DemoTest.cpp)

target_include_directories(docanTest PRIVATE include)

target_link_libraries(
    docanTest
    PRIVATE docanMock
            asyncMockImpl
            cpp2canMock
            transportMock
            utilMock
            bspMock
            etl
            gtest_main)

gtest_discover_tests(docanTest PROPERTIES LABELS "docanTest")
