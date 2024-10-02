add_library(bspDynamicClient INTERFACE)

target_include_directories(bspDynamicClient INTERFACE include)

target_link_libraries(bspDynamicClient INTERFACE bsp platform)
