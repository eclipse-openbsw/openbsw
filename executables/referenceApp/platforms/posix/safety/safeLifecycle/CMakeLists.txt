add_library(safeLifecycle src/safeLifecycle/SafetyManager.cpp)

target_include_directories(safeLifecycle PUBLIC include)

target_link_libraries(safeLifecycle PRIVATE safeSupervisor safeUtils)

if (BUILD_UNIT_TESTS)
    target_compile_definitions(safeLifecycle PUBLIC ETL_THROW_EXCEPTIONS)
endif ()
