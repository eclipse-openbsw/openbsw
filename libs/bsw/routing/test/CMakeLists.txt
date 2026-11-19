find_package(Python3 REQUIRED Interpreter)

set(_ROUTING_TOOLS_DIR ${CMAKE_SOURCE_DIR}/tools)
set(_BLOB_GENERATOR ${_ROUTING_TOOLS_DIR}/blob/generate_routing.py)
set(_ROUTING_TEST_INPUT ${CMAKE_CURRENT_SOURCE_DIR}/routing.jsonl)
set(_ROUTING_TEST_GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)

file(GLOB_RECURSE _BLOB_GENERATOR_MODULES CONFIGURE_DEPENDS
     ${_ROUTING_TOOLS_DIR}/blob/*.py)

set(_ROUTING_TEST_GENERATED_HEADERS
    ${_ROUTING_TEST_GENERATED_DIR}/blob/ConfigType.h
    ${_ROUTING_TEST_GENERATED_DIR}/blob/configuration.h
    ${_ROUTING_TEST_GENERATED_DIR}/routing/channelId.h
    ${_ROUTING_TEST_GENERATED_DIR}/routing/constants.h)

add_custom_command(
    OUTPUT ${_ROUTING_TEST_GENERATED_HEADERS}
    COMMAND
        ${CMAKE_COMMAND} -E env "PYTHONPATH=${_ROUTING_TOOLS_DIR}"
        ${Python3_EXECUTABLE} ${_BLOB_GENERATOR} ${_ROUTING_TEST_INPUT}
        ${_ROUTING_TEST_GENERATED_DIR}/blob
        ${_ROUTING_TEST_GENERATED_DIR}/routing
    DEPENDS ${_ROUTING_TEST_INPUT} ${_BLOB_GENERATOR_MODULES}
    COMMENT "Generating routing test configuration headers"
    VERBATIM)

add_custom_target(routingTestConfigurationGenerated
                  DEPENDS ${_ROUTING_TEST_GENERATED_HEADERS})

target_include_directories(routingConfiguration BEFORE
                           INTERFACE ${_ROUTING_TEST_GENERATED_DIR})

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

target_include_directories(routingTest BEFORE
                           PUBLIC ${_ROUTING_TEST_GENERATED_DIR})

add_dependencies(routingTest routingTestConfigurationGenerated)

target_link_libraries(
    routingTest
    PRIVATE routing
            ioMock
            cpp2ethernetMock
            bspMock
            gmock_main)

gtest_discover_tests(routingTest PROPERTIES LABELS "routingTest")
