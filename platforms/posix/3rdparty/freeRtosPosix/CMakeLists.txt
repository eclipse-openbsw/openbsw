add_library(freeRtosPosixPort INTERFACE)

target_include_directories(freeRtosPosixPort INTERFACE include)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_library(freeRtosPosix src/utils/wait_for_event.c src/port.c)

target_include_directories(freeRtosPosix PRIVATE include)

target_link_libraries(freeRtosPosix PUBLIC bspInterrupts freeRtos
                                           Threads::Threads)

target_compile_definitions(freeRtosPosix PRIVATE _POSIX_C_SOURCE=200112L)
