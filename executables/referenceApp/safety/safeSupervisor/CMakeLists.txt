add_library(safeSupervisor src/safeSupervisor/SafeSupervisor.cpp)

target_include_directories(safeSupervisor PUBLIC include)

target_link_libraries(
    safeSupervisor
    PUBLIC safeMonitor platform
    PRIVATE safeUtils common bspInterrupts)

if (PLATFORM_SUPPORT_IO)
    target_link_libraries(safeSupervisor PRIVATE bspIo safeIo)
endif ()

target_link_libraries(safeSupervisor PUBLIC bspMcu)
