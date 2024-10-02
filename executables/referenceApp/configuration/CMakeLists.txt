add_library(configuration INTERFACE)

target_include_directories(configuration INTERFACE include)

target_link_libraries(configuration INTERFACE common)

add_library(commonImpl common/src/busid/BusId.cpp)

target_link_libraries(commonImpl PRIVATE configuration)
