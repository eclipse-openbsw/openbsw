string(TOLOWER "${RP2XXX_CHIP}" _RP2XXX_CHIP_DIR)

add_library(freeRtosRP2xxxPort INTERFACE)

target_include_directories(freeRtosRP2xxxPort
                           INTERFACE ${_RP2XXX_CHIP_DIR}/include)

target_link_libraries(freeRtosRP2xxxPort INTERFACE pico-sdk)

add_library(freeRtosRP2xxx ${_RP2XXX_CHIP_DIR}/src/port.c)

target_include_directories(freeRtosRP2xxx PUBLIC ${_RP2XXX_CHIP_DIR}/include)

target_link_libraries(
    freeRtosRP2xxx
    PUBLIC freeRtos pico-sdk bspMcu
    PRIVATE bspInterrupts)

target_compile_options(freeRtosRP2xxx PRIVATE -Wno-unused-but-set-variable)
