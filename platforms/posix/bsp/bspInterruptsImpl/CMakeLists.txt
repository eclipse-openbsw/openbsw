if (NOT BUILD_EXECUTABLE STREQUAL "unitTest")
    set(bspInterruptsImplName "bspInterruptsImpl")
else ()
    # For unit tests, the mock implementation from bspInterrupts is used. To
    # still be able to add tests for *this* module, the target is renamed.
    set(bspInterruptsImplName "bspInterruptsImplUt")
endif ()

if (BUILD_TARGET_RTOS STREQUAL "FREERTOS")
    add_library(${bspInterruptsImplName}
                freertos/src/interrupts/suspendResumeAllInterrupts.cpp)
    target_include_directories(${bspInterruptsImplName} PUBLIC freertos/include)
    target_link_libraries(${bspInterruptsImplName} PRIVATE freeRtos)
elseif (BUILD_TARGET_RTOS STREQUAL "THREADX")
    add_library(${bspInterruptsImplName}
                threadx/src/interrupts/suspendResumeAllInterrupts.cpp)
    target_include_directories(${bspInterruptsImplName} PUBLIC threadx/include)
    target_link_libraries(${bspInterruptsImplName} PRIVATE threadX)
endif ()

target_link_libraries(${bspInterruptsImplName} PUBLIC platform)
