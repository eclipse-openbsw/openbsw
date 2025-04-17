add_library(
    startUp
    src/bsp/bootHeader.S
    src/bsp/startUp.S
    src/os/isr/isr_can.cpp
    src/os/isr/isr_erm.cpp
    src/os/isr/isr_ftfc.cpp
    src/os/isr/isr_sys.cpp
    src/os/isr/isr_wdg.cpp)

target_link_libraries(startUp PRIVATE bspMcu common hardFaultHandler)

add_library(main src/systems/BspSystem.cpp src/lifecycle/StaticBsp.cpp
                 src/systems/CanSystem.cpp src/main.cpp)

target_include_directories(main PUBLIC include)

target_link_libraries(
    main
    PRIVATE asyncConsole
            bsp
            bspClock
            bspCore
            bspMcuWatchdog
            canflex2Transceiver
            configuration
            freeRtosCm4SysTick
            lifecycle
            startUp
            watchdogManager
            safeLifecycle
            safeMemory
            safeWatchdog
    PUBLIC bspInputManager bspOutputManager)

add_library(osHooks src/osHooks.cpp)

target_include_directories(osHooks PRIVATE include)

target_link_libraries(osHooks PRIVATE common freeRtosCm4SysTick)

set_target_properties(
    startUp
    PROPERTIES PROP_LINKER_SCRIPT
               "${CMAKE_CURRENT_SOURCE_DIR}/linkerscript/application.dld")
