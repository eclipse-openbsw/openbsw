add_library(safeSupervisor src/safeSupervisor/SafeSupervisor.cpp)

target_include_directories(safeSupervisor PUBLIC include)

target_link_libraries(
    safeSupervisor
    PUBLIC safeMonitor platform
    PRIVATE safeUtils common bspInterrupts)

if (NOT BUILD_UNIT_TESTS)
    target_link_libraries(safeSupervisor PUBLIC bspMcu)
endif ()

if (BUILD_UNIT_TESTS)
    add_subdirectory(mock)
    target_include_directories(safeSupervisor PUBLIC mock/include)
    target_link_libraries(safeSupervisor PUBLIC safeSupervisorMock utbspMcu)
endif ()
