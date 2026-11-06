add_library(bspCan src/can/MCP2515.cpp)

target_include_directories(bspCan PUBLIC include)

target_link_libraries(bspCan PUBLIC bspConfiguration cpp2can etl pico-sdk)
