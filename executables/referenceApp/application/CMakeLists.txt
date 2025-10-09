add_executable(
    app.referenceApp
    src/app/app.cpp
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

if (PLATFORM_SUPPORT_STORAGE)
    target_sources(app.referenceApp PRIVATE src/systems/StorageSystem.cpp)
endif ()

if (PLATFORM_SUPPORT_ETHERNET)
    target_sources(app.referenceApp PRIVATE src/systems/EthernetSystem.cpp)
endif ()

set_target_properties(app.referenceApp PROPERTIES SUFFIX ".elf")

if (PLATFORM_SUPPORT_ROM_CHECK)
    add_custom_command(
        TARGET app.referenceApp
        POST_BUILD
        COMMAND python3 ${OPENBSW_DIR}/tools/safety/crcCheck.py
        COMMENT
            "Running post-build script to run the CRC and inject the result into the ELF file"
    )
endif ()

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
            util
            udsConfigurationImpl
            runtime
            safeLifecycle
            safeUtils
            safeWatchdog
            socBsp)

if (BUILD_TARGET_RTOS STREQUAL "FREERTOS")
    target_link_libraries(app.referenceApp PRIVATE osFreeRtos asyncFreeRtosImpl)
elseif (BUILD_TARGET_RTOS STREQUAL "THREADX")
    target_link_libraries(app.referenceApp PRIVATE osThreadX asyncThreadXImpl)
else ()
    message(
        FATAL_ERROR
            "The target_build_rtos '${BUILD_TARGET_RTOS}' is not included in the suppported rtoses."
    )
endif ()

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

if (PLATFORM_SUPPORT_STORAGE)
    target_link_libraries(app.referenceApp PRIVATE storage)
endif ()

if (PLATFORM_SUPPORT_ROM_CHECK)
    target_link_libraries(app.referenceApp PRIVATE safeMemory)
endif ()
