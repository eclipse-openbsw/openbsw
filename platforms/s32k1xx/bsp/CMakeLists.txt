add_subdirectory(bspAdc)
add_subdirectory(bspClock)
add_subdirectory(bspCore)
add_subdirectory(bspEepromDriver)
add_subdirectory(bspEthernet)
add_subdirectory(bspFlexCan)
add_subdirectory(bspFtm)
add_subdirectory(bspFtmPwm)
add_subdirectory(bspInterruptsImpl)

if (BUILD_EXECUTABLE STREQUAL "referenceApp")
    add_subdirectory(bspIo)
endif ()

add_subdirectory(bspMcu)
add_subdirectory(bspTja1101)
add_subdirectory(bspUart)
add_subdirectory(canflex2Transceiver)

add_library(socBsp INTERFACE)
target_link_libraries(
    socBsp
    INTERFACE bspAdc
              bspClock
              bspCore
              bspEepromDriver
              bspInterruptsImpl
              bspIo
              bspMcu
              safeBspMcuWatchdog
              bspUart)
