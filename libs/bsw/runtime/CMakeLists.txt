add_library(runtime src/runtime/StatisticsWriter.cpp)

if (BUILD_TRACING)
    target_sources(runtime PRIVATE src/runtime/Tracer.cpp)
endif ()

target_include_directories(runtime PUBLIC include)

target_link_libraries(runtime PUBLIC async bsp etl util)
