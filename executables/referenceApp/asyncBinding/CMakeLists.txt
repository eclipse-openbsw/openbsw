add_library(asyncBinding INTERFACE)

if (OPENBSW_TARGET_RTOS STREQUAL "freertos")
    target_include_directories(asyncBinding INTERFACE freertos/include)
elseif (OPENBSW_TARGET_RTOS STREQUAL "threadx")
    target_include_directories(asyncBinding INTERFACE threadx/include)
endif ()

target_link_libraries(asyncBinding INTERFACE runtime)
