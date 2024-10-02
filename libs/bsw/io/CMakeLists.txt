add_library(io src/io/BufferedWriter.cpp)

target_include_directories(io PUBLIC include)

target_link_libraries(io PUBLIC estd platform)

if (BUILD_UNIT_TESTS)
    add_library(ioMock INTERFACE)

    target_include_directories(ioMock INTERFACE mock/include)

    target_link_libraries(ioMock INTERFACE io gmock_main)
endif ()
