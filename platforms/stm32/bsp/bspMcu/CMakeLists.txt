enable_language(ASM)
add_library(bspMcu src/reset/softwareSystemReset.cpp ${STM32_STARTUP_ASM})

target_include_directories(
    bspMcu PUBLIC include ${CMAKE_SOURCE_DIR}/libs/3rdparty/cmsis
                  ${CMAKE_SOURCE_DIR}/libs/3rdparty/cmsis/m-profile)

if (STM32_FAMILY STREQUAL "F4")
    target_include_directories(bspMcu PUBLIC include/3rdparty/st/stm32f4)
elseif (STM32_FAMILY STREQUAL "G4")
    target_include_directories(bspMcu PUBLIC include/3rdparty/st/stm32g4)
endif ()

target_compile_definitions(
    bspMcu PUBLIC ${STM32_DEVICE_UPPER} STM32_FAMILY_${STM32_FAMILY}
                  CAN_TYPE_${CAN_TYPE})

target_link_libraries(bspMcu PUBLIC platform)
