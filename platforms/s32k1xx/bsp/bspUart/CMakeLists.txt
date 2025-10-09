add_library(bspUart src/UartSpecific.cpp)

target_include_directories(bspUart PUBLIC include)

target_link_libraries(bspUart PUBLIC bspIo etl PRIVATE bspConfiguration)
