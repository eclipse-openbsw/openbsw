set(_PICO_SDK_COMMON_DEFINITIONS
    PICO_NO_HARDWARE=0
    PICO_ON_DEVICE=1
    PICO_BUILD=1
    PICO_32BIT=1
    LIB_PICO_CLIB_INTERFACE=1
    LIB_PICO_CRT0=1
    LIB_PICO_STDIO_UART=1
    LIB_PICO_PRINTF=1
    LIB_PICO_PRINTF_PICO=1
    LIB_PICO_RUNTIME=1
    LIB_PICO_RUNTIME_INIT=1
    LIB_PICO_STDIO=1
    LIB_PICO_STDLIB=1
    LIB_PICO_UTIL=1
    LIB_PICO_NEWLIB_INTERFACE=1
    LIB_PICO_PLATFORM=1
    LIB_PICO_PLATFORM_COMMON=1
    LIB_PICO_PLATFORM_COMPILER=1)

# Header only view of the SDK. The chip subdirectory added at the bottom of this
# file appends the chip specific include directory and defines (PICO_RP2040 /
# PICO_RP2350, PICO_BOARD, ...), so everything linking against this target sees
# a complete SDK configuration.
add_library(pico-interface INTERFACE)

target_include_directories(pico-interface INTERFACE include)

target_compile_definitions(pico-interface
                           INTERFACE ${_PICO_SDK_COMMON_DEFINITIONS})

# Carrier target for the compiled SDK objects (crt0, runtime init, ...). They
# must end up on the final link line as plain object files: as static archive
# members they would not be pulled in in the right order (e.g. _fini referenced
# by libc). The generator expression makes sure only the linking executable
# picks them up, so intermediate static libraries do not absorb them.
add_library(pico-sdk-runtime INTERFACE)

target_link_libraries(pico-sdk-runtime INTERFACE pico-interface)

target_sources(
    pico-sdk-runtime
    INTERFACE
        "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:$<TARGET_OBJECTS:pico-sdk>>"
)

# Sources shared by all RP2xxx chips. The chip subdirectory appends its own
# sources to this target.
add_library(
    pico-sdk
    OBJECT
    src/bootrom.cpp
    src/claim.cpp
    src/clocks.cpp
    src/crt0.S
    src/exception.cpp
    src/exit.cpp
    src/gpio.cpp
    src/irq.cpp
    src/irq_handler_chain.S
    src/lock_core.cpp
    src/mutex.cpp
    src/newlib_interface.cpp
    src/panic.cpp
    src/pll.cpp
    src/printf.cpp
    src/runtime.cpp
    src/runtime_init.cpp
    src/runtime_init_clocks.cpp
    src/spi.cpp
    src/standard_binary_info.cpp
    src/stdio.cpp
    src/stdio_uart.cpp
    src/sync.cpp
    src/sync_spin_lock.cpp
    src/ticks.cpp
    src/time.cpp
    src/timer.cpp
    src/uart.cpp
    src/watchdog.cpp
    src/xosc.cpp)

target_include_directories(pico-sdk PUBLIC include)

target_compile_definitions(pico-sdk PUBLIC ${_PICO_SDK_COMMON_DEFINITIONS})

string(TOLOWER "${RP2XXX_CHIP}" _RP2XXX_CHIP_DIR)
add_subdirectory(${_RP2XXX_CHIP_DIR})
