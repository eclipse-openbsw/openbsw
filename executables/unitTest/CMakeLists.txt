cmake_minimum_required(VERSION 3.22 FATAL_ERROR)

set(OPENBSW_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    CACHE PATH "Path to Eclipse OpenBSW")

project(
    "Eclipse OpenBSW Unit Tests"
    DESCRIPTION
        "BSW workspace with reference application for multiple platforms"
    LANGUAGES CXX C ASM)

set(BUILD_TARGET_PLATFORM
    "POSIX"
    CACHE STRING "Target Platform")

add_compile_definitions(PLATFORM_SUPPORT_CAN=1)
add_compile_definitions(PLATFORM_SUPPORT_UDS=1)
add_compile_definitions(PLATFORM_SUPPORT_IO=1)

include(${OPENBSW_DIR}/CMakeLists.txt)

add_library(asyncPlatform ALIAS asyncMock)
add_library(os::freeRtos ALIAS freeRtosMock)

add_subdirectory(bsp)
add_subdirectory(common)
add_subdirectory(configuration)
add_subdirectory(transportConfiguration)
add_subdirectory(udsConfiguration)
add_subdirectory(bspConfiguration)
