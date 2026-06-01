if (NOT BUILD_EXECUTABLE STREQUAL "unitTest")
    if (NOT DEFINED STM32_CHIP)
        message(
            FATAL_ERROR
                "STM32_CHIP must be defined (e.g., STM32F413ZH or STM32G474RE)")
    endif ()

    if (STM32_CHIP STREQUAL "STM32F413ZH")
        include(${CMAKE_CURRENT_LIST_DIR}/cmake/stm32f413zh.cmake)
    elseif (STM32_CHIP STREQUAL "STM32G474RE")
        include(${CMAKE_CURRENT_LIST_DIR}/cmake/stm32g474re.cmake)
    else ()
        message(
            FATAL_ERROR
                "Unsupported STM32_CHIP: ${STM32_CHIP}. Supported: STM32F413ZH, STM32G474RE"
        )
    endif ()

    add_subdirectory(bsp)
endif ()
