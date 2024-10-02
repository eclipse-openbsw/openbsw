add_library(bspSystemTime src/bsp/time/systemTimer.cpp)

target_link_libraries(bspSystemTime PUBLIC asyncCoreConfiguration bsp platform)
