add_library(bspIo src/io/Gpio.cpp)
target_include_directories(bspIo PUBLIC include)
target_link_libraries(
    bspIo
    PUBLIC bspMcu
    PRIVATE platform)
