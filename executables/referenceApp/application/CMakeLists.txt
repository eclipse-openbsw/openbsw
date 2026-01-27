add_executable(
    app.referenceApp
    src/app/app.cpp
    src/console/console.cpp
    src/logger/logger.cpp
    src/systems/DemoSystem.cpp
    src/systems/RuntimeSystem.cpp
    src/systems/SysAdminSystem.cpp
    src/systems/SafetySystem.cpp
    src/main.cpp)

if (PLATFORM_SUPPORT_TRANSPORT)
    target_sources(app.referenceApp PRIVATE src/systems/TransportSystem.cpp)
    if (PLATFORM_SUPPORT_UDS)
        target_sources(app.referenceApp PRIVATE src/systems/UdsSystem.cpp
                                                src/uds/ReadIdentifierPot.cpp)
    endif ()
endif ()

if (PLATFORM_SUPPORT_ETHERNET)
    target_sources(app.referenceApp PRIVATE src/systems/EthernetSystem.cpp)
    if (PLATFORM_SUPPORT_TRANSPORT)
        target_sources(app.referenceApp
                       PRIVATE src/systems/DoIpServerSystem.cpp)
    endif ()
endif ()

if (PLATFORM_SUPPORT_CAN)
    target_sources(app.referenceApp PRIVATE src/app/CanDemoListener.cpp)
    if (PLATFORM_SUPPORT_TRANSPORT)
        target_sources(app.referenceApp PRIVATE src/systems/DoCanSystem.cpp)
    endif ()
endif ()

if (PLATFORM_SUPPORT_STORAGE)
    target_sources(app.referenceApp PRIVATE src/systems/StorageSystem.cpp)
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
            asyncConsole
            bspMcu
            configuration
            etl
            etlImpl
            lifecycle
            consoleCommands
            logger
            printf
            loggerIntegration
            main
            stdioConsoleInput
            osRtos
            asyncRtosImpl
            util
            runtime
            safeLifecycle
            safeUtils
            safeWatchdog
            bspUart
            socBsp)

if (PLATFORM_SUPPORT_TRANSPORT)
    target_link_libraries(app.referenceApp PRIVATE transport
                                                   transportRouterSimple)
    if (PLATFORM_SUPPORT_UDS)
        target_link_libraries(app.referenceApp PRIVATE uds udsConfigurationImpl)
    endif ()
endif ()

if (PLATFORM_SUPPORT_CAN)
    target_link_libraries(app.referenceApp PRIVATE cpp2can)
    if (PLATFORM_SUPPORT_TRANSPORT)
        target_link_libraries(app.referenceApp PRIVATE docan)
    endif ()
endif ()

if (PLATFORM_SUPPORT_ETHERNET)
    target_link_libraries(app.referenceApp PRIVATE cpp2ethernet lwipSocket
                                                   lwipcore ethConfiguration)
    if (PLATFORM_SUPPORT_TRANSPORT)
        target_link_libraries(app.referenceApp PRIVATE doip)
    endif ()
endif ()

if (PLATFORM_SUPPORT_STORAGE)
    target_link_libraries(app.referenceApp PRIVATE storage)
endif ()

if (PLATFORM_SUPPORT_ROM_CHECK)
    target_link_libraries(app.referenceApp PRIVATE safeMemory)
endif ()

if (BUILD_RUST)
    target_compile_definitions(app.referenceApp PRIVATE BUILD_RUST)
    # Link against Rust static library(should be only one static library)
    target_link_libraries(app.referenceApp PRIVATE rust_hello_world)
    # Add include directory for Rust C/C++ headers
    target_include_directories(
        app.referenceApp
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../rustHelloWorld/include)
endif ()
