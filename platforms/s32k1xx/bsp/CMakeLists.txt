if (BUILD_EXECUTABLE STREQUAL "referenceApp")
    add_subdirectory(bspAdc)
    add_subdirectory(bspClock)
    add_subdirectory(bspCore)
    add_subdirectory(bspEepromDriver)
    add_subdirectory(bspEthernet)
    add_subdirectory(bspFlexCan)
    add_subdirectory(bspInterruptsImpl)
    add_subdirectory(bspIo)
    add_subdirectory(bspMcu)
    add_subdirectory(bspTja1101)
    add_subdirectory(bspUart)
endif ()

add_subdirectory(bspFtm)
add_subdirectory(bspFtmPwm)
add_subdirectory(canflex2Transceiver)

add_library(socBsp INTERFACE)

if (BUILD_EXECUTABLE STREQUAL "referenceApp")
    target_link_libraries(
        socBsp
        INTERFACE bspIo
                  bspMcu
                  bspAdc
                  bspClock
                  bspCore
                  bspInterruptsImpl
                  bspEepromDriver
                  safeBspMcuWatchdog
                  bspUart)
endif ()
