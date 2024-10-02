add_library(bspMock src/bsp/timer/SystemTimerMock.cpp)

target_include_directories(bspMock PUBLIC include)

target_link_libraries(bspMock PUBLIC bsp estd gmock_main)
