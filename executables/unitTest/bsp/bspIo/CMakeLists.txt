add_library(bspIo src/io/Io.cpp)

target_include_directories(bspIo PUBLIC include test/include)

target_link_libraries(bspIo PUBLIC bsp bspConfiguration)
