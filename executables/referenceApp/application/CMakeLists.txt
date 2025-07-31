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
    if (PLATFORM_SUPPORT_ETHERNET)
        list(APPEND app.referenceAppExtraSources src/systems/EthernetSystem.cpp)
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
            lwipcore
            asyncConsole
            bspMcu
            configuration
            ethConfiguration
            etl
            lifecycle
            consoleCommands
            logger
            printf
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

if (PLATFORM_SUPPORT_ETHERNET)

    target_link_libraries(app.referenceApp PRIVATE cpp2ethernet lwipSocket)
endif ()
