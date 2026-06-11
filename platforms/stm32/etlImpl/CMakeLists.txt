add_library(etlImpl src/print.cpp src/clocks.cpp)

target_link_libraries(etlImpl PRIVATE etl bsp util)
