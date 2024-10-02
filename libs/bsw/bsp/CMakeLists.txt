add_library(bsp INTERFACE)

target_include_directories(bsp INTERFACE include)

target_link_libraries(bsp INTERFACE common estd platform)

if (BUILD_UNIT_TESTS)

    add_subdirectory(mock)

endif ()
