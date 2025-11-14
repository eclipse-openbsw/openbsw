add_library(bspUart src/bsp/Uart.cpp)

target_include_directories(bspUart PUBLIC include)

target_link_libraries(bspUart PRIVATE bsp platform)
