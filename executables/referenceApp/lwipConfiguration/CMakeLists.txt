add_library(lwipConfiguration src/rng.cpp)

target_include_directories(lwipConfiguration PUBLIC include)

target_link_libraries(lwipConfiguration PUBLIC lwipSysArch platform etl)
