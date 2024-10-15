add_subdirectory(bspEepromDriver)
add_subdirectory(bspInterruptsImpl)
add_subdirectory(bspMcu)
add_subdirectory(bspStdio)
add_subdirectory(bspSystemTime)
add_subdirectory(socketCanTransceiver)
add_subdirectory(tapEthernetDriver)

add_library(socBsp INTERFACE)
target_link_libraries(socBsp INTERFACE bspInterruptsImpl bspMcu bspStdio
                                       bspSystemTime)
