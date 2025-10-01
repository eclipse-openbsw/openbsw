add_library(utbspUart src/Uart.cpp)

target_include_directories(utbspUart PUBLIC include)

target_link_libraries(utbspUart PRIVATE platform etl)
