add_library(canTransceiver src/can/transceiver/can/CanTransceiver.cpp)

if (BUILD_EXECUTABLE STREQUAL "unitTest")
    target_include_directories(canTransceiver PUBLIC include test/mock/include)

    target_link_libraries(
        canTransceiver
        PUBLIC asyncMockImpl
               bsp
               cpp2can
               etl
               bspIo
               lifecycle
               gmock
               configuration)
else ()
    target_include_directories(canTransceiver PUBLIC include)

    target_link_libraries(canTransceiver PUBLIC async bspCan lifecycle)
endif ()
