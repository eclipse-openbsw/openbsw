add_library(shed src/multi_list.cpp src/ops_internal.cpp)

target_include_directories(shed PUBLIC include)

target_link_libraries(shed PUBLIC etl platform)
