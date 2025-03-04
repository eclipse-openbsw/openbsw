cmake_minimum_required(VERSION 3.22 FATAL_ERROR)

set(OPENBSW_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    CACHE PATH "Path to Eclipse OpenBSW")

project(
    "Eclipse OpenBSW ReferenceApp"
    DESCRIPTION
        "BSW workspace with reference application for multiple platforms"
    LANGUAGES CXX C ASM)

option(PLATFORM_SUPPORT_CAN "Turn CAN support on or off" OFF)
option(PLATFORM_SUPPORT_IO "Turn IO support on or off" OFF)
option(PLATFORM_SUPPORT_UDS "Turn UDS support on or off" OFF)

set(BUILD_TARGET_PLATFORM
    "POSIX"
    CACHE STRING "Target Platform")

set(SUPPORTED_PLATFORMS POSIX S32K148EVB)
if (NOT (BUILD_TARGET_PLATFORM IN_LIST SUPPORTED_PLATFORMS))
    message(FATAL_ERROR "Unknown target platform <${BUILD_TARGET_PLATFORM}>")
endif ()

message(STATUS "Target platform: <${BUILD_TARGET_PLATFORM}>")

if (BUILD_TARGET_PLATFORM STREQUAL "POSIX" AND NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(PLATFORM_SUPPORT_CAN
        ON
        CACHE BOOL "Turn CAN support on or off" FORCE)
    set(PLATFORM_SUPPORT_IO
        OFF
        CACHE BOOL "Turn IO support on or off" FORCE)
elseif (BUILD_TARGET_PLATFORM STREQUAL "S32K148EVB")
    set(PLATFORM_SUPPORT_CAN
        ON
        CACHE BOOL "Turn CAN support on or off" FORCE)
    set(PLATFORM_SUPPORT_IO
        ON
        CACHE BOOL "Turn IO support on or off" FORCE)
endif ()

if (PLATFORM_SUPPORT_CAN)
    set(PLATFORM_SUPPORT_UDS
        ON
        CACHE BOOL "Turn UDS support on or off" FORCE)
endif ()

if (PLATFORM_SUPPORT_CAN)
    add_compile_definitions(PLATFORM_SUPPORT_CAN=1)
endif ()

if (PLATFORM_SUPPORT_UDS)
    add_compile_definitions(PLATFORM_SUPPORT_UDS=1)
endif ()

if (PLATFORM_SUPPORT_IO)
    add_compile_definitions(PLATFORM_SUPPORT_IO=1)
endif ()

if (PLATFORM_SUPPORT_WATCHDOG)
    add_compile_definitions(PLATFORM_SUPPORT_WATCHDOG=1)
endif ()

include(${OPENBSW_DIR}/Filelists.cmake)

# Configure async and freeRtos libs depending on build type.
add_library(asyncPlatform ALIAS asyncFreeRtos)
add_library(osFreeRtos ALIAS freeRtos)

if (BUILD_TARGET_PLATFORM STREQUAL "POSIX")
    add_subdirectory(platforms/posix)
    add_library(freeRtosPort ALIAS freeRtosPosixPort)
    add_library(freeRtosPortImpl ALIAS freeRtosPosix)
elseif (BUILD_TARGET_PLATFORM STREQUAL "S32K148EVB")
    add_subdirectory(platforms/s32k148evb)
    add_library(freeRtosPort ALIAS freeRtosCm4SysTickPort)
    add_library(freeRtosPortImpl ALIAS freeRtosCm4SysTick)
endif ()

add_subdirectory(application)
add_subdirectory(asyncBinding)
add_subdirectory(asyncCoreConfiguration)
add_subdirectory(configuration)
add_subdirectory(consoleCommands)
add_subdirectory(udsConfiguration)
add_subdirectory(transportConfiguration)
