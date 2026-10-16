# RP2350 specific part of the vendored pico-sdk.
#
# Unlike the RP2040 the RP2350 bootrom does not export the v1 math shims, so no
# bootrom backed integer, memory or floating point routines are vendored here.
# The Cortex-M33 has a hardware FPU and the compiler / newlib implementations
# are used instead, which is why the RP2350 board options wrap far fewer
# symbols than the RP2040 ones.

# The concrete board is selected by the board options via RP2XXX_BOARD, which
# names one of the headers in include/boards.
if (NOT RP2XXX_BOARD)
    set(RP2XXX_BOARD "pico2")
endif ()

set(_PICO_SDK_RP2350_DEFINITIONS
    PICO_RP2350=1 PICO_ARM=1 PICO_RISCV=0 PICO_BOARD=\"${RP2XXX_BOARD}\"
    PICO_BOARD_HEADER=\"boards/${RP2XXX_BOARD}.h\")

target_include_directories(pico-interface INTERFACE include)
target_compile_definitions(pico-interface
                           INTERFACE ${_PICO_SDK_RP2350_DEFINITIONS})

target_include_directories(pico-sdk PUBLIC include)
target_compile_definitions(pico-sdk PUBLIC ${_PICO_SDK_RP2350_DEFINITIONS})

add_subdirectory(src/boot_stage2)
