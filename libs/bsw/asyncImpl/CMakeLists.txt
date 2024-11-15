add_library(asyncImpl INTERFACE)

target_include_directories(asyncImpl INTERFACE include)

target_link_libraries(asyncImpl INTERFACE etl platform)

if (BUILD_UNIT_TESTS)

    add_library(asyncImplMock INTERFACE)

    target_include_directories(asyncImplMock INTERFACE mock/include)

    target_link_libraries(asyncImplMock INTERFACE gmock platform)

endif ()
