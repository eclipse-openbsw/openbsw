cmake_minimum_required(VERSION 3.22 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_STANDARD 99)

project(
    "Eclipse OpenBSW"
    VERSION 0.1.0
    DESCRIPTION
        "BSW workspace with reference application for multiple platforms"
    LANGUAGES CXX C ASM)

option(BUILD_UNIT_TESTS "Build unit tests" OFF)

add_compile_options(
    "$<$<COMPILE_LANG_AND_ID:CXX,Clang,GNU>:-O2;-g3;-Werror;-Wall;-Wvla;-Woverloaded-virtual>"
)

add_compile_options(
    "$<$<COMPILE_LANG_AND_ID:C,Clang,GNU>:-O2;-g3;-Werror;-Wall>")

#[[
    Build type is unspecified by default, but in case it was specified, we're
    forcing debug to have same flags as release, as difference in optimized and
    unoptimized version of software in embedded systems can be crucial.
#]]
set(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_RELEASE})

if (BUILD_UNIT_TESTS)
    add_compile_definitions(UNIT_TEST=1)
    include(GoogleTest)
    include(CTest)
    enable_testing()
endif ()

set(OPENBSW_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
    CACHE PATH "Path to Eclipse OpenBSW")
add_subdirectory(${OPENBSW_DIR}/libs openbsw/libs)
add_subdirectory(${OPENBSW_DIR}/platforms openbsw/platforms)
