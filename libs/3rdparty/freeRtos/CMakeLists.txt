add_library(
    freeRtos
    src/croutine.c
    src/event_groups.c
    src/list.c
    src/queue.c
    src/stream_buffer.c
    src/tasks.c
    src/timers.c)

target_include_directories(freeRtos PUBLIC include)

target_link_libraries(
    freeRtos
    PUBLIC platform freeRtosConfiguration freeRtosPort
    PRIVATE freeRtosPortImpl osHooks)

target_compile_options(freeRtos PRIVATE -Wno-unused-variable)

if (BUILD_EXECUTABLE STREQUAL "unitTest")
    add_subdirectory(mock)
endif ()
