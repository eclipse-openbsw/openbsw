# In the unit test build the drivers below are replaced by the host stubs in
# executables/unitTest/bsp, so only the modules that are actually exercised by a
# test are added here.
if (NOT BUILD_EXECUTABLE STREQUAL "unitTest")
    add_subdirectory(bspAdc)
    add_subdirectory(bspClock)
    add_subdirectory(bspCore)
    add_subdirectory(bspEepromDriver)
    add_subdirectory(bspEthernet)
    add_subdirectory(bspCan)
    add_subdirectory(bspFtm)
    add_subdirectory(bspFtmPwm)
    add_subdirectory(bspInterruptsImpl)
    add_subdirectory(bspIo)
    add_subdirectory(bspMcu)
    add_subdirectory(bspSci)
    add_subdirectory(bspUart)
endif ()

add_subdirectory(canTransceiver)

add_library(socBsp INTERFACE)

if (NOT BUILD_EXECUTABLE STREQUAL "unitTest")
    target_link_libraries(
        socBsp
        INTERFACE bspAdc
                  bspClock
                  bspCore
                  bspEepromDriver
                  bspInterruptsImpl
                  bspIo
                  bspMcu
                  bspSci
                  bspUart
                  safeBspMcuWatchdog)
endif ()
