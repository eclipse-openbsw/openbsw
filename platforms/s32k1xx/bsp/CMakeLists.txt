add_subdirectory(bspAdc)
add_subdirectory(bspClock)
add_subdirectory(bspCore)
add_subdirectory(bspFlexCan)
add_subdirectory(bspFtm)
add_subdirectory(bspFtmPwm)
add_subdirectory(bspInterruptsImpl)
add_subdirectory(bspIo)
add_subdirectory(bspMcu)
add_subdirectory(bspMcuWatchdog)
add_subdirectory(bspSci)
add_subdirectory(canflex2Transceiver)

add_library(socBsp INTERFACE)
target_link_libraries(
    socBsp
    INTERFACE bspAdc
              bspClock
              bspCore
              bspInterruptsImpl
              bspIo
              bspMcu
              bspMcuWatchdog
              bspSci)
