add_library(
    safeLifecycle
    src/safeLifecycle/SafetyManager.cpp src/safeLifecycle/IsrHooks.cpp
    src/safeLifecycle/IsrWrapper.cpp)

target_include_directories(safeLifecycle PUBLIC include)

target_link_libraries(
    safeLifecycle
    PUBLIC platform
    PRIVATE safeSupervisor
            safeSystem
            safeWatchdog
            safeUtils
            safeMemory
            util
            bsp
            bspInterrupts
            startUp)

if (BUILD_UNIT_TESTS)
    target_compile_definitions(safeLifecycle PUBLIC ESTD_HAS_EXCEPTIONS
                                                    ESTL_ASSERT_MESSAGE_ALL
                                                    ETL_THROW_EXCEPTIONS)
endif ()
