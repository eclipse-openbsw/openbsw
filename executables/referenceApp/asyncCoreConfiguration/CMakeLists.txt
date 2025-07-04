add_library(asyncCoreConfiguration INTERFACE)

if (OPENBSW_TARGET_RTOS STREQUAL "freertos")
    target_include_directories(asyncCoreConfiguration
                               INTERFACE freertos/include)
elseif (OPENBSW_TARGET_RTOS STREQUAL "threadx")
    target_include_directories(asyncCoreConfiguration INTERFACE threadx/include)
endif ()

target_link_libraries(asyncCoreConfiguration INTERFACE common)
