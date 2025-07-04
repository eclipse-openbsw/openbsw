cmake_minimum_required(VERSION 3.22 FATAL_ERROR)

set(OPENBSW_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    CACHE PATH "Path to Eclipse OpenBSW")

set(OPENBSW_APP_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
    CACHE PATH "Path to Eclipse OpenBSW Application")

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
set(BUILD_TARGET_RTOS
    "FREERTOS"
    CACHE STRING "Target RTOS")

# Convert target name to lower case to match directory name
string(TOLOWER "${BUILD_TARGET_PLATFORM}" OPENBSW_TARGET_PLATFORM)
string(TOLOWER "${BUILD_TARGET_RTOS}" OPENBSW_TARGET_RTOS)

set(OPENBSW_TARGET_PLATFORM_DIR
    ${OPENBSW_APP_DIR}/platforms/${OPENBSW_TARGET_PLATFORM})

include(${OPENBSW_TARGET_PLATFORM_DIR}/Options.cmake)

set(OPENBSW_PLATFORM_DIR ${OPENBSW_DIR}/platforms/${OPENBSW_PLATFORM})
message(STATUS "OPENBSW_APP_DIR: ${OPENBSW_APP_DIR}")
message(STATUS "OPENBSW_TARGET_PLATFORM: ${OPENBSW_TARGET_PLATFORM}")
message(STATUS "OPENBSW_TARGET_RTOS: ${OPENBSW_TARGET_RTOS}")
message(STATUS "Resolved OPENBSW_TARGET_PLATFORM_DIR: ${OPENBSW_TARGET_PLATFORM_DIR}")
message(STATUS "Resolved OPENBSW_PLATFORM_DIR: ${OPENBSW_PLATFORM_DIR}")
message(STATUS "Target platform: <${BUILD_TARGET_PLATFORM}>")
message(STATUS "Target rtos: <${BUILD_TARGET_RTOS}>")

if (PLATFORM_SUPPORT_CAN)
    set(PLATFORM_SUPPORT_UDS
        ON
        CACHE BOOL "Turn UDS support on or off" FORCE)
endif ()

include(${OPENBSW_DIR}/Filelists.cmake)

add_subdirectory(${OPENBSW_TARGET_PLATFORM_DIR})

# Configure async and Rtos libs depending on target rtos.
if (OPENBSW_TARGET_RTOS STREQUAL "freertos")
    add_compile_definitions(SUPPORT_FREERTOS)
    add_library(asyncPlatform ALIAS asyncFreeRtos)
    add_library(osFreeRtos ALIAS freeRtos)
elseif (OPENBSW_TARGET_RTOS STREQUAL "threadx")
    add_compile_definitions(SUPPORT_THREADX)
    add_library(asyncPlatform ALIAS asyncThreadX)
endif ()

add_subdirectory(application)
add_subdirectory(asyncBinding)
add_subdirectory(asyncCoreConfiguration)
add_subdirectory(configuration)
add_subdirectory(consoleCommands)
add_subdirectory(udsConfiguration)
add_subdirectory(transportConfiguration)
