add_library(runtime src/runtime/StatisticsWriter.cpp)

target_include_directories(runtime PUBLIC include)

target_link_libraries(runtime PUBLIC async estd util)
