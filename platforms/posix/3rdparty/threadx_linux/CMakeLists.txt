add_library(threadXLinuxPort INTERFACE)

target_include_directories(threadXLinuxPort INTERFACE inc)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_library(
    threadXLinux
    ${CMAKE_CURRENT_LIST_DIR}/src/tx_initialize_low_level.c
    ${CMAKE_CURRENT_LIST_DIR}/src/tx_thread_context_restore.c
    ${CMAKE_CURRENT_LIST_DIR}/src/tx_thread_context_save.c
    ${CMAKE_CURRENT_LIST_DIR}/src/tx_thread_interrupt_control.c
    ${CMAKE_CURRENT_LIST_DIR}/src/tx_thread_schedule.c
    ${CMAKE_CURRENT_LIST_DIR}/src/tx_thread_stack_build.c
    ${CMAKE_CURRENT_LIST_DIR}/src/tx_thread_system_return.c
    ${CMAKE_CURRENT_LIST_DIR}/src/tx_timer_interrupt.c)

target_include_directories(threadXLinux PUBLIC ${CMAKE_CURRENT_LIST_DIR}/inc)

target_link_libraries(
    threadXLinux PUBLIC bspInterrupts threadX threadXCoreConfiguration
                        Threads::Threads)

target_compile_options(threadXLinuxPort INTERFACE -DTX_INCLUDE_USER_DEFINE_FILE)
target_compile_options(threadXLinux PRIVATE -DTX_INCLUDE_USER_DEFINE_FILE)

target_compile_options(threadXLinux PRIVATE --warn-no-unused-result)
