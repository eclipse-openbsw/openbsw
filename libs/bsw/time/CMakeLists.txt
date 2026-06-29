add_library(time src/time/TimestampProvider.cpp)

target_include_directories(time PUBLIC include)

target_link_libraries(time PRIVATE bsp)
