set(app.referenceAppExtraSources)

if (PLATFORM_SUPPORT_UDS)
    list(
        APPEND
        app.referenceAppExtraSources
        src/systems/TransportSystem.cpp
        src/systems/UdsSystem.cpp
        src/uds/ReadIdentifierPot.cpp)

    if (PLATFORM_SUPPORT_CAN)
        list(APPEND app.referenceAppExtraSources src/systems/DoCanSystem.cpp)
    endif ()
endif ()

add_executable(
    app.referenceApp
    src/app/app.cpp
    src/app/CanDemoListener.cpp
    src/logger/logger.cpp
    src/console/console.cpp
    src/systems/DemoSystem.cpp
    src/systems/RuntimeSystem.cpp
    src/systems/SysAdminSystem.cpp
    src/systems/SafetySystem.cpp
    ${app.referenceAppExtraSources}
    src/main.cpp)

if (BUILD_BENCHMARK)
    if (BUILD_TARGET_PLATFORM STREQUAL "POSIX")
        message(FATAL_ERROR "Benchmarking not supported on POSIX")
    endif ()
    target_sources(app.referenceApp PRIVATE src/systems/BenchmarkSystem.cpp)
endif ()

set_target_properties(app.referenceApp PROPERTIES SUFFIX ".elf")

if (TARGET startUp)
    get_target_property(LINKER_SCRIPT startUp PROP_LINKER_SCRIPT)

    if (DEFINED LINKER_SCRIPT)
        target_link_options(app.referenceApp PRIVATE -T ${LINKER_SCRIPT})
    endif ()
endif ()

target_include_directories(app.referenceApp PRIVATE include)

target_link_libraries(
    app.referenceApp
    PRIVATE asyncBinding
            asyncConsole
            bspMcu
            configuration
            estd
            lifecycle
            consoleCommands
            logger
            loggerIntegration
            main
            stdioConsoleInput
            osFreeRtos
            asyncFreeRtosImpl
            util
            udsConfigurationImpl
            runtime
            safeLifecycle
            socBsp)

if (PLATFORM_SUPPORT_UDS)

    target_link_libraries(app.referenceApp PRIVATE transport
                                                   transportRouterSimple uds)

endif ()

if (PLATFORM_SUPPORT_CAN)

    target_link_libraries(app.referenceApp PRIVATE cpp2can docan)
endif ()
