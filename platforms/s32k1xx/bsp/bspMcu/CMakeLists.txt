add_library(bspMcu src/reset/softwareSystemReset.cpp)

target_include_directories(bspMcu PUBLIC include)

target_link_libraries(bspMcu PUBLIC platform)

if (BUILD_UNIT_TESTS)
    add_library(bspMcuMock mock/gmock/mcu/mcuMock.cpp)

    target_include_directories(bspMcuMock PUBLIC /mock/include)

    target_link_libraries(bspMcuMock PRIVATE bspMcu bspMcuTest gmock_main)
endif ()
