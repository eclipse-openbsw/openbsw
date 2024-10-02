add_library(bspError src/BspError.cpp)

target_include_directories(bspError PUBLIC include)

target_link_libraries(bspError bsp bspInterrupts)
