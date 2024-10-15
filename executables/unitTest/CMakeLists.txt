cmake_minimum_required(VERSION 3.22 FATAL_ERROR)

set(OPENBSW_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    CACHE PATH "Path to Eclipse OpenBSW")

set(OPENBSW_APP_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
    CACHE PATH "Path to Eclipse OpenBSW Application")

set(OPENBSW_PLATFORM_DIR
    ${OPENBSW_DIR}/platforms/posix
    CACHE PATH "Path to Eclipse OpenBSW Platform")

project(
    "Eclipse OpenBSW Unit Tests"
    DESCRIPTION
        "BSW workspace with reference application for multiple platforms"
    LANGUAGES CXX C ASM)

set(BUILD_TARGET_PLATFORM
    "POSIX"
    CACHE STRING "Target Platform")

include(${OPENBSW_DIR}/Filelists.cmake)

add_library(asyncPlatform ALIAS asyncMock)
add_library(osFreeRtos ALIAS freeRtosMock)

add_subdirectory(bsp)
add_subdirectory(common)
add_subdirectory(configuration)
add_subdirectory(transportConfiguration)
add_subdirectory(udsConfiguration)
add_subdirectory(bspConfiguration)
