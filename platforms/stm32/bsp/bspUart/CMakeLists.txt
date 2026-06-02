if (BUILD_EXECUTABLE STREQUAL "unitTest")
    # Header-only for unit tests (no hardware-dependent Uart.cpp)
    add_library(bspUart INTERFACE)
    target_include_directories(bspUart INTERFACE include)
    target_link_libraries(bspUart INTERFACE bsp)
else ()
    add_library(bspUart src/Uart.cpp)
    target_include_directories(bspUart PUBLIC include)
    # bspMcu must be PUBLIC: the public header bsp/UartParams.h includes
    # mcu/mcu.h and uses device types, so consumers need bspMcu's include paths
    # and device-macro compile definitions.
    target_link_libraries(bspUart PUBLIC bsp bspMcu)
endif ()
