add_library(
    cpp2can
    src/can/canframes/CANFrame.cpp
    src/can/CanLogger.cpp
    src/can/filter/AbstractStaticBitFieldFilter.cpp
    src/can/filter/BitFieldFilter.cpp
    src/can/filter/IntervalFilter.cpp
    src/can/transceiver/AbstractCANTransceiver.cpp)

target_include_directories(cpp2can PUBLIC include)

target_link_libraries(
    cpp2can
    PUBLIC common etl util
    PRIVATE bsp bspInterrupts)

if (BUILD_EXECUTABLE STREQUAL "unitTest")
    add_library(cpp2canMock
                mock/src/can/transceiver/AbstractCANTransceiverMock.cpp)

    target_include_directories(cpp2canMock PUBLIC mock/include)

    target_link_libraries(cpp2canMock PRIVATE bspMock cpp2can gmock_main)
endif ()
