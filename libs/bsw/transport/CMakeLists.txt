add_library(
    transport
    src/transport/AbstractTransportLayer.cpp
    src/transport/DestituteTransportMessageProvider.cpp
    src/transport/LogicalAddress.cpp
    src/transport/QueuedTransportLayer.cpp
    src/transport/SimpleTransportMessageProvider.cpp
    src/transport/TransportLogger.cpp
    src/transport/TransportMessage.cpp)

target_include_directories(transport PUBLIC include)

target_link_libraries(transport PUBLIC common estd util async)

if (BUILD_UNIT_TESTS)
    add_library(
        transportMock
        mock/gmock/src/transport/TransportMessageProvidingListenerMock.cpp)

    target_include_directories(transportMock PUBLIC mock/gmock/include)

    target_link_libraries(transportMock PRIVATE gmock transport)

endif ()
