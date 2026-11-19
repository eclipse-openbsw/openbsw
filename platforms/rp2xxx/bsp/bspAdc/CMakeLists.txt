add_library(bspAdc src/tester/AnalogTester.cpp)

target_include_directories(bspAdc PUBLIC include)

target_link_libraries(bspAdc PRIVATE bspConfiguration util)
