add_library(commonImpl common/src/common/mock/busid/BusId.cpp)

target_include_directories(commonImpl PRIVATE common/include)

target_link_libraries(commonImpl PRIVATE common)

# Empty dummy lib for unit tests
add_library(configuration INTERFACE)
