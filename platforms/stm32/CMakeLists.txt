# Copyright 2024 Contributors to the Eclipse Foundation
#
# SPDX-License-Identifier: EPL-2.0

# STM32 platform for OpenBSW
# Supports STM32F4 (bxCAN) and STM32G4 (FDCAN) families via STM32_CHIP selection.

if (NOT BUILD_EXECUTABLE STREQUAL "unitTest")
    # --- Chip selection ---
    if (NOT DEFINED STM32_CHIP)
        message(FATAL_ERROR "STM32_CHIP must be defined (e.g., STM32F413ZH or STM32G474RE)")
    endif ()

    if (STM32_CHIP STREQUAL "STM32F413ZH")
        include(${CMAKE_CURRENT_LIST_DIR}/cmake/stm32f413zh.cmake)
    elseif (STM32_CHIP STREQUAL "STM32G474RE")
        include(${CMAKE_CURRENT_LIST_DIR}/cmake/stm32g474re.cmake)
    else ()
        message(FATAL_ERROR "Unsupported STM32_CHIP: ${STM32_CHIP}. Supported: STM32F413ZH, STM32G474RE")
    endif ()

    # BSP modules
    add_subdirectory(bsp)

    # ETL implementation
    add_subdirectory(etlImpl)
endif ()
