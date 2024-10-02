add_library(bspOutputManager src/outputManager/Output.cpp
                             src/outputManager/OutputTester.cpp)

target_include_directories(bspOutputManager PUBLIC include)

target_link_libraries(bspOutputManager PUBLIC bspDynamicClient bspIo estd util)
