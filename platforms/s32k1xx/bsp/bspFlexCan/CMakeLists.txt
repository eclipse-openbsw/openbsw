add_library(bspFlexCan src/can/FlexCANDevice.cpp)

target_include_directories(bspFlexCan PUBLIC include)

target_link_libraries(bspFlexCan PUBLIC bsp bspConfiguration cpp2can etl)
