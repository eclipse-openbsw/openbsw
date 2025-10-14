cmake_minimum_required(VERSION 3.28 FATAL_ERROR)

if (CMAKE_CXX_STANDARD AND NOT ${CMAKE_CXX_STANDARD} EQUAL 14)
    message(WARNING "Using custom CXX standard (C++${CMAKE_CXX_STANDARD})")
else ()
    set(CMAKE_CXX_STANDARD 14)
endif ()

if (CMAKE_C_STANDARD AND NOT ${CMAKE_C_STANDARD} EQUAL 99)
    message(WARNING "Using custom C standard (C${CMAKE_C_STANDARD})")
else ()
    set(CMAKE_C_STANDARD 99)
endif ()

set(BUILD_TARGET_PLATFORM
    "POSIX"
    CACHE STRING "Target Platform")

# Convert target name to lower case to match directory name
string(TOLOWER "${BUILD_TARGET_PLATFORM}" OPENBSW_TARGET)
include(executables/referenceApp/platforms/${OPENBSW_TARGET}/Options.cmake)

message(STATUS "Target platform: <${BUILD_TARGET_PLATFORM}>")

# Set preprocessor defines based on cmake options by the same name to configure
# platform features.
get_cmake_property(_all_vars VARIABLES)
foreach (var ${_all_vars})
    if (var MATCHES "^PLATFORM_SUPPORT_")
        if (${var})
            add_compile_definitions(${var}=1)
            message(STATUS "${var} ON")
        endif ()
    endif ()
endforeach ()

project(
    "Eclipse OpenBSW"
    VERSION 0.1.0
    DESCRIPTION
        "BSW workspace with reference application for multiple platforms"
    LANGUAGES CXX C ASM)

list(APPEND CMAKE_MODULE_PATH "cmake/modules")

option(BUILD_UNIT_TESTS "Build unit tests" OFF)

option(BUILD_TRACING "Build CTF tracing" OFF)

add_compile_options(
    #[[
        Supressing stringop-overflow and maybe-uninitialized, as they are often
        false-positive.
    ]]
    "$<$<COMPILE_LANG_AND_ID:CXX,GNU>:-Wno-error=stringop-overflow;-Wno-error=maybe-uninitialized>"
)

if (BUILD_UNIT_TESTS)
    add_compile_definitions(UNIT_TEST=1)
    include(GoogleTest)
    include(CTest)
    include(CodeCoverage)
    append_coverage_compiler_flags()
    enable_testing()

    set(OPENBSW_APP_DIR "${CMAKE_CURRENT_LIST_DIR}/executables/unitTest")
    set(OPENBSW_PLATFORM_DIR "${CMAKE_CURRENT_LIST_DIR}/platforms/posix")
endif ()

if (BUILD_REFERENCE)
    set(OPENBSW_APP_DIR "${CMAKE_CURRENT_LIST_DIR}/executables/referenceApp")
    if (BUILD_TARGET_PLATFORM STREQUAL "S32K148EVB")
        set(OPENBSW_PLATFORM_DIR "${CMAKE_CURRENT_LIST_DIR}/platforms/s32k1xx")
    else ()
        set(OPENBSW_PLATFORM_DIR "${CMAKE_CURRENT_LIST_DIR}/platforms/posix")
    endif ()
endif ()

if (BUILD_TRACING)
    add_compile_definitions(TRACING=1)
    if (TRACING_BUFFER_SIZE)
        add_compile_definitions(TRACING_BUFFER_SIZE=${TRACING_BUFFER_SIZE})
    endif ()
endif ()

set(INCLUDE_OPENBSW_LIBS_BSP
    ON
    CACHE BOOL "Include openbsw/libs/bsp/ in build")

add_subdirectory(platforms EXCLUDE_FROM_ALL)
add_subdirectory(libs EXCLUDE_FROM_ALL)

if (BUILD_UNIT_TESTS)
    add_subdirectory(libs/bsw/async/test)
    add_subdirectory(libs/bsw/asyncConsole/test)
    add_subdirectory(libs/bsw/asyncFreeRtos/test)
    add_subdirectory(libs/bsw/asyncImpl/examples)
    add_subdirectory(libs/bsw/asyncImpl/test)
    add_subdirectory(libs/bsw/bsp/test)
    add_subdirectory(libs/bsw/cpp2can/test)
    add_subdirectory(libs/bsw/cpp2ethernet/test)
    add_subdirectory(libs/bsw/docan/test)
    add_subdirectory(libs/bsw/io/examples)
    add_subdirectory(libs/bsw/io/test)
    add_subdirectory(libs/bsw/lifecycle/examples)
    add_subdirectory(libs/bsw/lifecycle/test)
    add_subdirectory(libs/bsw/logger/test)
    add_subdirectory(libs/bsw/lwipSocket/test)
    add_subdirectory(libs/bsw/platform/test)
    add_subdirectory(libs/bsw/runtime/test)
    add_subdirectory(libs/bsw/storage/test)
    add_subdirectory(libs/bsw/timer/test)
    add_subdirectory(libs/bsw/transport/test)
    add_subdirectory(libs/bsw/uds/test)
    add_subdirectory(libs/bsw/util/test)

    add_subdirectory(libs/bsp/bspInputManager/test)
    add_subdirectory(libs/bsp/bspCharInputOutput/test)
    add_subdirectory(libs/bsp/bspDynamicClient/test)
    add_subdirectory(libs/bsp/bspInterrupts/test)
    add_subdirectory(libs/bsp/bspOutputManager/test)
    add_subdirectory(libs/bsp/bspOutputPwm/test)

    add_subdirectory(libs/safety/safeUtils/test)
    add_subdirectory(libs/safety/safeMonitor/test)

    add_subdirectory(platforms/posix/bsp/bspEepromDriver/test)
    add_subdirectory(platforms/posix/bsp/socketCanTransceiver)
    add_subdirectory(platforms/posix/bsp/socketCanTransceiver/test)
    add_subdirectory(platforms/s32k1xx/bsp/bspCore/test)
    add_subdirectory(platforms/s32k1xx/bsp/bspEepromDriver/test)
    add_subdirectory(platforms/s32k1xx/bsp/bspIo/test)
    add_subdirectory(platforms/s32k1xx/bsp/bspSci/test)
    add_subdirectory(platforms/s32k1xx/bsp/canflex2Transceiver)
    add_subdirectory(platforms/s32k1xx/bsp/canflex2Transceiver/test)

    add_subdirectory(executables/referenceApp/safety/safeSupervisor EXCLUDE_FROM_ALL)
    add_subdirectory(executables/referenceApp/safety/safeSupervisor/test)

    add_subdirectory(executables/unitTest)
    target_include_directories(etl BEFORE INTERFACE executables/unitTest/etl_profile)
elseif(BUILD_REFERENCE)
    target_include_directories(etl BEFORE INTERFACE executables/referenceApp/etl_profile)
    add_subdirectory(executables/referenceApp)
endif()
