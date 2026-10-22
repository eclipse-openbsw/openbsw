add_executable(
    middlewareSomeIpTest
    src/AllocatorSelector.cpp src/PortRange.cpp
    src/middleware/bindings/someip/SomeIpBridgeTest.cpp
    src/middleware/bindings/someip/SomeIpCodecTest.cpp)

target_link_libraries(
    middlewareSomeIpTest
    PRIVATE middlewareSomeIp
            cpp2someip
            middleware
            middlewareTestSupport
            bspMock
            cpp2someipMock
            cpp2ethernetMock
            utCommon
            utilMock
            asyncMockImpl
            gmock_main)

gtest_discover_tests(middlewareSomeIpTest PROPERTIES LABELS
                                                     "middlewareSomeIpTest")
