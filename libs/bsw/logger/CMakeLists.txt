add_library(logger src/logger/DefaultLoggerCommand.cpp
                   src/logger/EntrySerializer.cpp)

target_include_directories(logger PUBLIC include)

target_link_libraries(logger PUBLIC util)

if (BUILD_EXECUTABLE STREQUAL "unitTest")
    add_library(loggerMock INTERFACE)

    target_include_directories(loggerMock INTERFACE mock/include)

    target_link_libraries(loggerMock INTERFACE bspMock utilMock gmock_main)
endif ()
