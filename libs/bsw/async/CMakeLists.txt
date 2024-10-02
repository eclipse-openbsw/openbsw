add_library(async INTERFACE)

target_include_directories(async INTERFACE include)

target_link_libraries(async INTERFACE asyncPlatform)

if (BUILD_UNIT_TESTS)

    add_subdirectory(mock)

endif ()
