cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

# Set up the project
project(threadXRP2xxx LANGUAGES C ASM)

add_library(threadXRP2xxx)

add_library(threadXRP2xxxPort INTERFACE)

if (RP2XXX_CHIP STREQUAL "RP2040")
    set(_TX_PORT_DIR ports/cortex_m0/gnu)
else ()
    set(_TX_PORT_DIR ports/cortex_m33/gnu)
endif ()

add_subdirectory(${_TX_PORT_DIR})

# The Cortex-M33 port ships a tx_initialize_low_level.S of its own, but it
# hardcodes the SysTick reload value, expects the symbols _vectors and
# __RAM_segment_used_end__ that this platform does not define, and never calls
# setupApplicationsIsr. The board provides its own implementation instead, in
# the same way the Cortex-M4 platforms do, so the vendored one is dropped here
# rather than patched.
if (RP2XXX_CHIP STREQUAL "RP2350")
    get_target_property(_TX_SOURCES threadXRP2xxx SOURCES)
    list(
        REMOVE_ITEM
        _TX_SOURCES
        "${CMAKE_CURRENT_LIST_DIR}/${_TX_PORT_DIR}/src/tx_initialize_low_level.S"
    )
    set_target_properties(threadXRP2xxx PROPERTIES SOURCES "${_TX_SOURCES}")
endif ()

target_include_directories(threadXRP2xxxPort INTERFACE ${_TX_PORT_DIR}/inc)

target_link_libraries(threadXRP2xxx PUBLIC bspInterrupts bspMcu threadX
                                           threadXCoreConfiguration)
target_link_libraries(threadXRP2xxxPort INTERFACE threadXCoreConfiguration)

target_compile_definitions(threadXRP2xxxPort
                           INTERFACE TX_INCLUDE_USER_DEFINE_FILE)
target_compile_definitions(threadXRP2xxx PRIVATE TX_INCLUDE_USER_DEFINE_FILE)

# The RP2350 is run without TrustZone, so ThreadX is built for a single (secure)
# mode and the secure stack management of the Cortex-M33 port is compiled out.
if (RP2XXX_CHIP STREQUAL "RP2350")
    target_compile_definitions(threadXRP2xxxPort
                               INTERFACE TX_SINGLE_MODE_SECURE)
    target_compile_definitions(threadXRP2xxx PUBLIC TX_SINGLE_MODE_SECURE)
endif ()
