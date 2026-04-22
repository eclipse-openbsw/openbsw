add_library(bspMcu src/reset/softwareSystemReset.cpp)

target_include_directories(
    bspMcu
    PUBLIC include
           ${CMAKE_SOURCE_DIR}/libs/3rdparty/cmsis
           ${CMAKE_SOURCE_DIR}/libs/3rdparty/cmsis/m-profile)

target_link_libraries(bspMcu PUBLIC platform)
