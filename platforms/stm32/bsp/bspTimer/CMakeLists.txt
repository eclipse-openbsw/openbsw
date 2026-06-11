add_library(bspTimer src/bsp/SystemTimer/SystemTimer.cpp)

target_link_libraries(bspTimer PUBLIC bsp bspMcu bspInterrupts)
