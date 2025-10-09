add_library(asyncBinding INTERFACE)

# Configure asyncBinding depending on selected target rtos.
if (BUILD_TARGET_RTOS STREQUAL "FREERTOS")
    target_compile_definitions(asyncBinding INTERFACE SUPPORT_FREERTOS)
elseif (BUILD_TARGET_RTOS STREQUAL "THREADX")
    target_compile_definitions(asyncBinding INTERFACE SUPPORT_THREADX)
endif ()

target_include_directories(asyncBinding INTERFACE include)

target_link_libraries(asyncBinding INTERFACE runtime)
