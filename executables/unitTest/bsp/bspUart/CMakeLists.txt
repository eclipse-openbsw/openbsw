add_library(utbspUart src/bsp/Uart.cpp)

target_include_directories(utbspUart PUBLIC include)

target_link_libraries(utbspUart PRIVATE bsp bspConfiguration)
