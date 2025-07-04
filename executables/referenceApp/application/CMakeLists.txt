add_executable(
    app.referenceApp
    src/app/CanDemoListener.cpp
    src/console/console.cpp
    src/logger/logger.cpp
    src/systems/DemoSystem.cpp
    src/systems/RuntimeSystem.cpp
    src/systems/SysAdminSystem.cpp
    src/systems/SafetySystem.cpp
    src/main.cpp)

if (PLATFORM_SUPPORT_UDS)
    target_sources(
        app.referenceApp
        PRIVATE src/systems/TransportSystem.cpp src/systems/UdsSystem.cpp
                src/uds/ReadIdentifierPot.cpp)
endif ()

if (PLATFORM_SUPPORT_CAN)
    target_sources(app.referenceApp PRIVATE src/systems/DoCanSystem.cpp)
endif ()

if (OPENBSW_TARGET_RTOS STREQUAL "freertos")
    target_sources(app.referenceApp PRIVATE src/app/app-freertos.cpp)
elseif (OPENBSW_TARGET_RTOS STREQUAL "threadx")
    target_sources(app.referenceApp PRIVATE src/app/app-threadx.cpp
                                            src/systems/BackgroundSystem.cpp)
else ()
    message(
        FATAL_ERROR
            "The target_build_rtos '${OPENBSW_TARGET_RTOS}' is not included in the suppported rtoses."
    )
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
            etl
            lifecycle
            consoleCommands
            logger
            loggerIntegration
            main
            stdioConsoleInput
            util
            udsConfigurationImpl
            runtime
            safeLifecycle
            socBsp)

if (OPENBSW_TARGET_RTOS STREQUAL "freertos")
    target_link_libraries(app.referenceApp PRIVATE osFreeRtos asyncFreeRtosImpl)
elseif (OPENBSW_TARGET_RTOS STREQUAL "threadx")
    target_link_libraries(app.referenceApp PRIVATE threadX)
else ()
    message(
        FATAL_ERROR
            "The target_build_rtos '${OPENBSW_TARGET_RTOS}' is not included in the suppported rtoses."
    )
endif ()

if (PLATFORM_SUPPORT_UDS)
    target_link_libraries(app.referenceApp PRIVATE transport
                                                   transportRouterSimple uds)
endif ()

if (PLATFORM_SUPPORT_CAN)
    target_link_libraries(app.referenceApp PRIVATE cpp2can docan)
endif ()
