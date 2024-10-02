add_executable(
    transportTest
    src/transport/AbstractTransportLayerTest.cpp
    src/transport/DestitudeTransportMessageProviderTest.cpp
    src/transport/FunctionalTransportMessageSendJobTest.cpp
    src/transport/IncludeTest.cpp
    src/transport/QueuedTransportLayerTest.cpp
    src/transport/SimpleTransportMessageProviderTest.cpp
    src/transport/TesterAddressTest.cpp
    src/transport/TransportJobTest.cpp
    src/transport/TransportMessageTest.cpp
    ../src/transport/TransportConfiguration.cpp
    ../src/Logger.cpp)

target_include_directories(transportTest PRIVATE ../include)

target_link_libraries(
    transportTest
    PRIVATE transport
            asyncMockImpl
            gmock_main
            transportMock
            utilMock)

gtest_discover_tests(transportTest PROPERTIES LABELS "transportTest")
