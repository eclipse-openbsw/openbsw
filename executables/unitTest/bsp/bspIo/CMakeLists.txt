add_library(utbspIo src/io/Io.cpp)

target_include_directories(utbspIo PUBLIC include test/include)

target_link_libraries(utbspIo PUBLIC bsp)
