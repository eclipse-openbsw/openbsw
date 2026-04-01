add_library(logger src/logger/DefaultLoggerCommand.cpp
                   src/logger/EntrySerializer.cpp)

target_include_directories(logger PUBLIC include)

target_link_libraries(logger PUBLIC util)

add_library(openbsw_logger rust/include/BswLogger.h)
target_include_directories(openbsw_logger INTERFACE rust/include)
set_target_properties(openbsw_logger PROPERTIES LINKER_LANGUAGE CXX)

if (BUILD_EXECUTABLE STREQUAL "unitTest")
    add_library(loggerMock INTERFACE)

    target_include_directories(loggerMock INTERFACE mock/include)

    target_link_libraries(loggerMock INTERFACE bspMock utilMock gmock_main)
endif ()
