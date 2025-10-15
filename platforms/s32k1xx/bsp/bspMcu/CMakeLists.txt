add_library(bspMcu src/reset/softwareSystemReset.cpp)

target_include_directories(bspMcu PUBLIC include)

target_link_libraries(bspMcu PUBLIC platform)
