# RP2040 specific part of the vendored pico-sdk.

# The concrete board is selected by the board options via RP2XXX_BOARD, which
# names one of the headers in include/boards.
if (NOT RP2XXX_BOARD)
    set(RP2XXX_BOARD "pico")
endif ()

set(_PICO_SDK_RP2040_DEFINITIONS
    PICO_RP2040=1 PICO_BOARD=\"${RP2XXX_BOARD}\"
    PICO_BOARD_HEADER=\"boards/${RP2XXX_BOARD}.h\")

target_include_directories(pico-interface INTERFACE include)
target_compile_definitions(pico-interface
                           INTERFACE ${_PICO_SDK_RP2040_DEFINITIONS})

target_include_directories(pico-sdk PUBLIC include)
target_compile_definitions(pico-sdk PUBLIC ${_PICO_SDK_RP2040_DEFINITIONS})

# The RP2040 bootrom provides optimized integer, memory and floating point
# routines. They are wrapped in via -Wl,--wrap in the board options.
target_sources(
    pico-sdk
    PRIVATE src/bit_ops_aeabi.S
            src/divider_hardware.S
            src/double_aeabi_rp2040.S
            src/double_init_rom_rp2040.cpp
            src/double_v1_rom_shim_rp2040.S
            src/float_aeabi_rp2040.S
            src/float_init_rom_rp2040.cpp
            src/float_v1_rom_shim_rp2040.S
            src/mem_ops_aeabi.S
            src/pico_int64_ops_aeabi.S)

add_subdirectory(src/boot_stage2)
