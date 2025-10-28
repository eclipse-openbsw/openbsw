add_library(
    bspCharInputOutput
    src/charInputOutput/bspIo.cpp src/charInputOutput/charIo.cpp
    src/charInputOutput/charIoSerial.cpp)

target_include_directories(bspCharInputOutput PUBLIC include)

target_link_libraries(bspCharInputOutput PUBLIC bsp bspConfiguration printf)

if (BUILD_UNIT_TESTS)
    add_library(bspCharInputOutputMock INTERFACE)

    target_include_directories(bspCharInputOutputMock INTERFACE mock/include)

    target_link_libraries(bspCharInputOutputMock INTERFACE bspCharInputOutput)

    target_link_libraries(bspCharInputOutput PUBLIC utbspUart)

else ()
    target_link_libraries(bspCharInputOutput PUBLIC bspUart)

endif ()
