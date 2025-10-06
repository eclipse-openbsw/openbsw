add_library(safeLifecycle src/safeLifecycle/SafetyManager.cpp)

target_include_directories(safeLifecycle PUBLIC include)

target_link_libraries(
    safeLifecycle
    PRIVATE safeSupervisor safeUtils)

if (BUILD_UNIT_TESTS)
    target_compile_definitions(safeLifecycle PUBLIC ESTD_HAS_EXCEPTIONS
                                                    ESTL_ASSERT_MESSAGE_ALL
                                                    ETL_THROW_EXCEPTIONS)
endif ()
