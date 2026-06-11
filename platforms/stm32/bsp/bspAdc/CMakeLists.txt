add_library(bspAdc src/adc/Adc.cpp)
target_include_directories(bspAdc PUBLIC include)
target_link_libraries(
    bspAdc
    PUBLIC bspMcu
    PRIVATE platform)
