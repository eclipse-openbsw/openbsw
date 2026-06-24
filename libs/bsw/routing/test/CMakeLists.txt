# How to generate the test blob and its header files:
# python -m blob binary -i routing.jsonl -c blob.routing.table | python -m blob
# header data -n CONFIGURATION_BLOB > include/blob/configuration.h
# python -m blob.routing header ${BLOB_INPUT_FILE} > include/routing/channelId.h
# python -m blob header config-type > include/blob/ConfigType.h

target_include_directories(routingConfiguration INTERFACE include)

add_executable(
    routingTest
    src/routing/definition.cpp
    src/io/udp/SenderTest.cpp
    src/io/udp/ReceiverTest.cpp
    src/routing/PduTransportBufferedWriterTest.cpp
    src/routing/ChannelNamesTest.cpp
    src/routing/PduTransportIntegrationTest.cpp
    src/routing/RxAdapterTableTest.cpp
    src/routing/TxAdapterTableTest.cpp
    src/routing/PduRoutingTableTest.cpp
    src/routing/PduTransportRxAdapterTest.cpp
    src/routing/ClampedCounterTest.cpp
    src/routing/IntegrationTest.cpp
    src/routing/LegacyRxAdapterTest.cpp
    src/routing/RxAdapterTest.cpp
    src/routing/PduTransportConfigTest.cpp
    src/routing/HeaderTest.cpp
    src/routing/RouterTest.cpp
    src/routing/LegacyTxAdapterTest.cpp
    src/routing/PduTransportTxAdapterTest.cpp)

target_include_directories(routingTest PUBLIC include)

target_link_libraries(
    routingTest
    PRIVATE routing
            ioMock
            cpp2ethernetMock
            bspMock
            gmock_main)

gtest_discover_tests(routingTest PROPERTIES LABELS "routingTest")
