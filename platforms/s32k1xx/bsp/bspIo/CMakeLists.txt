add_library(bspIo src/io/Io.cpp)

target_include_directories(bspIo PUBLIC include)

target_link_libraries(
    bspIo
    PUBLIC bsp
           bspConfiguration # TODO: should this be split to ioConfiguration?
           bspMcu
           etl
           platform)

if (BUILD_EXECUTABLE STREQUAL "unitTest")
    add_library(bspIoMock mock/gmock/src/IoMock.cpp)

    target_include_directories(bspIoMock PUBLIC mock/gmock/include)

    target_link_libraries(bspIoMock PRIVATE gmock_main)
endif ()
