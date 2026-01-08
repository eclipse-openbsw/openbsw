add_library(io src/io/BufferedWriter.cpp)

target_include_directories(io PUBLIC include)

target_link_libraries(io PUBLIC etl platform util)

if (BUILD_EXECUTABLE STREQUAL "unitTest")
    add_library(ioMock INTERFACE)

    target_include_directories(ioMock INTERFACE mock/include)

    target_link_libraries(ioMock INTERFACE io etl gmock_main)
endif ()
