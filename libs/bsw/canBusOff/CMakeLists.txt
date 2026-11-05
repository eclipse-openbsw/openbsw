add_library(canBusOff src/canBusOff/CanBusOffPolicy.cpp)

target_include_directories(canBusOff PUBLIC include)

target_link_libraries(canBusOff PUBLIC etl platform)
