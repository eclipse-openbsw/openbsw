add_library(estd src/estd/assert.cpp src/estd/chrono.cpp src/estd/string.cpp
                 src/estd/string_view.cpp)

target_include_directories(estd PUBLIC include)

target_link_libraries(estd PUBLIC platform)

if (BUILD_UNIT_TESTS)

    target_compile_definitions(estd PUBLIC ESTD_HAS_EXCEPTIONS
                                           ESTL_ASSERT_MESSAGE_ALL)

    add_library(estdMock INTERFACE)

    target_include_directories(estdMock INTERFACE mock/include)

endif ()
