add_library(async INTERFACE)

target_include_directories(async INTERFACE include)

target_link_libraries(async INTERFACE asyncPlatform asyncBinding)

if (BUILD_EXECUTABLE STREQUAL "unitTest")

    add_subdirectory(mock)

endif ()
