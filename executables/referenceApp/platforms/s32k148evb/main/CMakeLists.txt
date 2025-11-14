add_library(startUpAsm OBJECT src/bsp/bootHeader.S src/bsp/startUp.S)

set_target_properties(startUpAsm PROPERTIES COMPILE_WARNING_AS_ERROR OFF)

add_library(
    startUp
    "$<TARGET_OBJECTS:startUpAsm>"
    src/os/isr/isr_can.cpp
    src/os/isr/isr_enet.cpp
    src/os/isr/isr_erm.cpp
    src/os/isr/isr_ftfc.cpp
    src/os/isr/isr_sys.cpp
    src/os/isr/isr_wdg.cpp)

if (BUILD_TARGET_RTOS STREQUAL "THREADX")
    target_sources(startUpAsm PRIVATE src/bsp/threadx/tx_initialize_low_level.S)
    target_sources(startUp PRIVATE src/bsp/threadx/tx_execution_initialize.c)
    target_link_libraries(startUpAsm PRIVATE asyncCoreConfiguration)
endif ()

target_link_libraries(
    startUp
    PRIVATE bspMcu
            common
            hardFaultHandler
            safeLifecycle
            safeMemory)

add_library(
    main
    src/systems/BspSystem.cpp
    src/lifecycle/StaticBsp.cpp
    src/systems/S32K148EvbEthernetSystem.cpp
    src/systems/CanSystem.cpp
    src/main.cpp)

target_include_directories(main PUBLIC include)

target_link_libraries(
    main
    PRIVATE asyncConsole
            bsp
            bspClock
            bspCore
            bspTja1101
            bspUart
            canflex2Transceiver
            configuration
            ethConfiguration
            etl
            lifecycle
            startUp
            safeBspMcuWatchdog
            safeLifecycle
            safeSupervisor
            safeMemory
            safeWatchdog
    PUBLIC bspInputManager bspOutputManager)

if (BUILD_TARGET_RTOS STREQUAL "FREERTOS")
    target_link_libraries(main PRIVATE freeRtosCm4SysTick)
    add_library(osHooks src/osHooks/freertos/osHooks.cpp)
    target_include_directories(osHooks PRIVATE include)
    target_link_libraries(osHooks PRIVATE common freeRtosCm4SysTick)
elseif (BUILD_TARGET_RTOS STREQUAL "THREADX")
    target_link_libraries(main PRIVATE threadXCortexM4)
    add_library(osHooks src/osHooks/threadx/osHooks.cpp)
    target_include_directories(osHooks PRIVATE include/osHooks/threadx)
    target_link_libraries(osHooks PRIVATE common threadXCortexM4 asyncThreadX
                                          threadXConfiguration)
endif ()

set_target_properties(
    startUp
    PROPERTIES PROP_LINKER_SCRIPT
               "${CMAKE_CURRENT_SOURCE_DIR}/linkerscript/application.dld")
set_target_properties(
    startUp
    PROPERTIES LINK_DEPENDS
               "${CMAKE_CURRENT_SOURCE_DIR}/linkerscript/application.dld")
