add_library(bspStdio src/bsp/stdIo/stdIo.cpp)

target_link_libraries(bspStdio PRIVATE bsp bspConfiguration bspUart platform)
