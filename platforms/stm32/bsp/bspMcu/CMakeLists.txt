# Copyright 2024 Contributors to the Eclipse Foundation
#
# SPDX-License-Identifier: EPL-2.0

enable_language(ASM)
add_library(bspMcu src/reset/softwareSystemReset.cpp ${STM32_STARTUP_ASM})

target_include_directories(bspMcu PUBLIC include include/3rdparty/cmsis
                                         include/3rdparty/cmsis/m-profile)

# Chip-family-specific CMSIS Device headers
if (STM32_FAMILY STREQUAL "F4")
    target_include_directories(bspMcu PUBLIC include/3rdparty/st/stm32f4)
elseif (STM32_FAMILY STREQUAL "G4")
    target_include_directories(bspMcu PUBLIC include/3rdparty/st/stm32g4)
endif ()

# Chip defines as PUBLIC so they propagate to all consumers (FreeRTOS, BSP, app)
target_compile_definitions(
    bspMcu PUBLIC ${STM32_DEVICE_UPPER} STM32_FAMILY_${STM32_FAMILY}
                  CAN_TYPE_${CAN_TYPE})

target_link_libraries(bspMcu PUBLIC platform)
