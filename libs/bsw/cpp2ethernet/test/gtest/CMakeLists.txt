add_executable(
    cpp2ethernetTest
    src/ip/NetworkInterfaceConfigTest.cpp
    src/ip/IPAddressTest.cpp
    src/ip/to_strTest.cpp
    src/ip/IPEndpointTest.cpp
    src/include_test.cpp
    src/udp/socket/AbstractDatagramSocketTest.cpp
    src/udp/DatagramPacketTest.cpp
    src/tcp/socket/AbstractSocketTest.cpp
    src/tcp/socket/AbstractServerSocketTest.cpp
    src/logger/Logger.cpp)

target_include_directories(cpp2ethernetTest PRIVATE include ../../mock/include ../../../util/mock/include)

target_link_libraries(cpp2ethernetTest PRIVATE cpp2ethernet bspMock gtest_main gmock utilMock asyncMock asyncMockImpl)

gtest_discover_tests(cpp2ethernetTest PROPERTIES LABELS "cpp2ethernetTest")
