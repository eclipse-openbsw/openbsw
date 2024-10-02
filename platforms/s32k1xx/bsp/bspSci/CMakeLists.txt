add_library(bspSci src/SciDevice.cpp)

target_include_directories(bspSci PUBLIC include)

target_link_libraries(bspSci PUBLIC bspIo estd)
