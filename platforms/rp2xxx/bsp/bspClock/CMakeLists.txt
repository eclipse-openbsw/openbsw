add_library(bspClock src/clockConfig.cpp)

target_include_directories(bspClock PUBLIC include)

target_link_libraries(bspClock PRIVATE bspConfiguration bspMcu)
