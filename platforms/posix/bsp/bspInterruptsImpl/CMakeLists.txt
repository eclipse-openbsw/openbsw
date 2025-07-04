if (OPENBSW_TARGET_RTOS STREQUAL "freertos")
    add_library(bspInterruptsImpl
                freertos/src/interrupts/suspendResumeAllInterrupts.cpp)
    target_include_directories(bspInterruptsImpl PUBLIC freertos/include)
    target_link_libraries(bspInterruptsImpl PRIVATE freeRtos)
elseif (OPENBSW_TARGET_RTOS STREQUAL "threadx")
    add_library(bspInterruptsImpl
                threadx/src/interrupts/suspendResumeAllInterrupts.cpp)
    target_include_directories(bspInterruptsImpl PUBLIC threadx/include)
    target_link_libraries(bspInterruptsImpl PRIVATE threadX)
endif ()

target_link_libraries(bspInterruptsImpl PUBLIC platform)
