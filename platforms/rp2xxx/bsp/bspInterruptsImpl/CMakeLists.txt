add_library(bspInterruptsImpl INTERFACE)

target_include_directories(bspInterruptsImpl INTERFACE include)

if (BUILD_TARGET_RTOS STREQUAL "FREERTOS")
    target_include_directories(bspInterruptsImpl INTERFACE freertos/include)
elseif (BUILD_TARGET_RTOS STREQUAL "THREADX")
    target_include_directories(bspInterruptsImpl INTERFACE threadx/include)
    target_link_libraries(bspInterruptsImpl INTERFACE threadX)
endif ()

target_link_libraries(bspInterruptsImpl INTERFACE platform)
