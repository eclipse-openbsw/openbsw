add_library(bspUart src/Uart.cpp)

target_include_directories(bspUart PUBLIC include)

target_link_libraries(
    bspUart
    PUBLIC bsp bspIo etl
    PRIVATE bspConfiguration)
