add_executable(
    app.integrationTest
    src/app/app.cpp
    src/logger/logger.cpp
    src/console/console.cpp
    src/systems/RuntimeSystem.cpp
    src/systems/SysAdminSystem.cpp
    src/main.cpp)

target_include_directories(app.integrationTest PRIVATE include)

target_link_libraries(
    app.integrationTest
    PRIVATE asyncBinding
            asyncConsole
            bspMcu
            etl
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
            bspUart
            socBsp)

set_target_properties(app.integrationTest PROPERTIES SUFFIX ".elf")

if (TARGET startUp)
    get_target_property(LINKER_SCRIPT startUp PROP_LINKER_SCRIPT)

    if (DEFINED LINKER_SCRIPT)
        target_link_options(app.integrationTest PRIVATE -T ${LINKER_SCRIPT})
    endif ()
endif ()
