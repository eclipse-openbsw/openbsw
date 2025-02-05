add_library(transport src/AbstractTransportLayer.cpp src/LogicalAddress.cpp
                      src/TransportLogger.cpp src/TransportMessage.cpp)

target_include_directories(transport PUBLIC include)

target_link_libraries(transport PUBLIC common estd util async)

if (BUILD_UNIT_TESTS)
    add_library(transportMock
                mock/gmock/src/TransportMessageProvidingListenerMock.cpp)

    target_include_directories(transportMock PUBLIC mock/gmock/include)

    target_link_libraries(transportMock PRIVATE gmock transport)

endif ()
