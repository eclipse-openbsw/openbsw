add_library(safeMonitor INTERFACE)

target_include_directories(safeMonitor INTERFACE include)

target_link_libraries(safeMonitor INTERFACE platform)

if (BUILD_UNIT_TESTS)

    target_compile_definitions(safeMonitor INTERFACE ESTD_HAS_EXCEPTIONS
                                                     ESTL_ASSERT_MESSAGE_ALL
                                                     ETL_THROW_EXCEPTIONS)

endif ()
