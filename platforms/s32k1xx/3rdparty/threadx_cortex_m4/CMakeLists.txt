add_library(threadXCortexM4Port INTERFACE)

target_include_directories(threadXCortexM4Port INTERFACE inc)

add_library(
    threadXCortexM4
    ${CMAKE_CURRENT_LIST_DIR}/src/interrupt_manager.c
    ${CMAKE_CURRENT_LIST_DIR}/src/tx_thread_context_restore.S
    ${CMAKE_CURRENT_LIST_DIR}/src/tx_thread_context_save.S
    ${CMAKE_CURRENT_LIST_DIR}/src/tx_thread_interrupt_control.S
    ${CMAKE_CURRENT_LIST_DIR}/src/tx_thread_schedule.S
    ${CMAKE_CURRENT_LIST_DIR}/src/tx_thread_stack_build.S
    ${CMAKE_CURRENT_LIST_DIR}/src/tx_thread_system_return.S
    ${CMAKE_CURRENT_LIST_DIR}/src/tx_timer_interrupt.S)

target_include_directories(threadXCortexM4 PUBLIC ${CMAKE_CURRENT_LIST_DIR}/inc)

target_link_libraries(threadXCortexM4 PUBLIC bspInterrupts bspMcu threadX threadXCoreConfiguration)

target_compile_definitions(threadXCortexM4Port INTERFACE -DTX_INCLUDE_USER_DEFINE_FILE=1)
target_compile_definitions(threadXCortexM4 PRIVATE -DTX_INCLUDE_USER_DEFINE_FILE=1)
