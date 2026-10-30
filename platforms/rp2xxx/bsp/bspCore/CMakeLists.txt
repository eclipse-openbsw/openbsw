add_library(bspCore src/cache/cache.cpp)

target_include_directories(bspCore PUBLIC include)

target_link_libraries(bspCore PRIVATE bspMcu)
