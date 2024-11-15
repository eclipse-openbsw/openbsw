add_library(bsp INTERFACE)

target_include_directories(bsp INTERFACE include)

target_link_libraries(bsp INTERFACE common platform etl)

if (BUILD_UNIT_TESTS)

    add_subdirectory(mock)

endif ()
