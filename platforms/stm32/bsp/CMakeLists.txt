add_subdirectory(bspMcu)
add_subdirectory(bspClock)
add_subdirectory(bspInterruptsImpl)
add_subdirectory(bspUart)
add_subdirectory(bspTimer)
add_subdirectory(bspIo)

# Aggregated BSP target
add_library(socBsp INTERFACE)
target_link_libraries(
    socBsp
    INTERFACE bspClock
              bspInterruptsImpl
              bspIo
              bspMcu
              bspTimer
              bspUart)
