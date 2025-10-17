if (NOT BUILD_UNIT_TESTS)
    set(bspInterruptsImplName "bspInterruptsImpl")
else ()
    # For unit tests, the mock implementation from bspInterrupts is used. To
    # still be able to add tests for *this* module, the target is renamed.
    set(bspInterruptsImplName "bspInterruptsImplUt")
endif ()

add_library(${bspInterruptsImplName} src/interrupt_manager.c)

target_include_directories(${bspInterruptsImplName} PUBLIC include)

if (BUILD_TARGET_RTOS STREQUAL "FREERTOS")
    target_include_directories(${bspInterruptsImplName} PUBLIC freertos/include)
elseif (BUILD_TARGET_RTOS STREQUAL "THREADX")
    target_include_directories(${bspInterruptsImplName} PUBLIC threadx/include)
    target_link_libraries(${bspInterruptsImplName} PUBLIC threadX)
endif ()

target_link_libraries(${bspInterruptsImplName} PRIVATE bspMcu platform)
