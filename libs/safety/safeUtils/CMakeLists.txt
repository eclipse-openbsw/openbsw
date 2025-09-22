add_library(safeUtils INTERFACE)

target_include_directories(safeUtils INTERFACE include)

target_link_libraries(safeUtils INTERFACE util)

if (BUILD_UNIT_TESTS)
    target_compile_definitions(safeUtils INTERFACE ESTD_HAS_EXCEPTIONS
                                                   ESTL_ASSERT_MESSAGE_ALL
                                                   ETL_THROW_EXCEPTIONS)
endif ()
