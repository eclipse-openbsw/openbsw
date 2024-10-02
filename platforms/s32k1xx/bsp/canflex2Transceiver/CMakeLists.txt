add_library(canflex2Transceiver
            src/can/transceiver/canflex2/CanFlex2Transceiver.cpp)

if (BUILD_UNIT_TESTS)
    target_include_directories(canflex2Transceiver PUBLIC include
                                                          test/mock/include)

    target_link_libraries(
        canflex2Transceiver
        PUBLIC asyncMockImpl
               bsp
               cpp2can
               utbspIo
               canflex2TransceiverMock
               lifecycle
               gmock)
else ()
    target_include_directories(canflex2Transceiver PUBLIC include)

    target_link_libraries(canflex2Transceiver PUBLIC async bspFlexCan lifecycle)
endif ()
