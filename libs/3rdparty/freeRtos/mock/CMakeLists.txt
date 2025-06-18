add_library(freeRtosMock src/os/FreeRtosMock.cpp)

target_include_directories(freeRtosMock PUBLIC include)

target_link_libraries(freeRtosMock PUBLIC etl freeRtosConfiguration gmock
                                          platform)
