# The 3rdparty pico-sdk, the FreeRTOS port and the startup code are target
# specific, so they are left out of the host unit test build. The chip selection
# is only relevant for those, the BSP modules below are chip independent.
if (NOT BUILD_EXECUTABLE STREQUAL "unitTest")
    if (NOT DEFINED RP2XXX_CHIP)
        message(FATAL_ERROR "RP2XXX_CHIP must be defined (RP2040 or RP2350)")
    endif ()

    if (NOT RP2XXX_CHIP STREQUAL "RP2040" AND NOT RP2XXX_CHIP STREQUAL "RP2350")
        message(FATAL_ERROR "Unsupported RP2XXX_CHIP: ${RP2XXX_CHIP}")
    endif ()

    message(STATUS "Target chip: <${RP2XXX_CHIP}>")

    # 3rdparty modules
    add_subdirectory(3rdparty/pico-sdk)

    if (BUILD_TARGET_RTOS STREQUAL "FREERTOS")
        add_subdirectory(3rdparty/freertos_rp2xxx)
    elseif (BUILD_TARGET_RTOS STREQUAL "THREADX")
        add_subdirectory(3rdparty/threadx)
    endif ()

    # Safety modules
    add_subdirectory(safety)

    # ETL implementation
    add_subdirectory(etlImpl)

    # Other modules
    add_subdirectory(hardFaultHandler)
    add_subdirectory(lwipSysArch)
endif ()

# BSP modules
add_subdirectory(bsp)
