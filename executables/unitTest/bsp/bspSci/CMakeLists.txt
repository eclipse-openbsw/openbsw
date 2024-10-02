add_library(utbspSci src/sci/SciDevice.cpp)

target_include_directories(utbspSci PUBLIC include)

target_link_libraries(utbspSci PRIVATE platform)
