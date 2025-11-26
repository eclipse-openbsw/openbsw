add_executable(
    transportTest
    src/AbstractTransportLayerTest.cpp
    src/IncludeTest.cpp
    src/TesterAddressTest.cpp
    src/TransportMessageTest.cpp
    src/TransportConfiguration.cpp
    src/Logger.cpp)

target_include_directories(transportTest PRIVATE include)

target_link_libraries(
    transportTest
    PRIVATE transport
            asyncMockImpl
            gmock_main
            transportMock
            utilMock
            etl)

gtest_discover_tests(transportTest PROPERTIES LABELS "transportTest")
