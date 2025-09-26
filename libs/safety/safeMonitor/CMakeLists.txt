add_library(safeMonitor INTERFACE)

target_include_directories(safeMonitor INTERFACE include)

target_link_libraries(safeMonitor INTERFACE platform common)

if (BUILD_UNIT_TESTS)

    target_compile_definitions(safeMonitor INTERFACE ETL_THROW_EXCEPTIONS)

endif ()
