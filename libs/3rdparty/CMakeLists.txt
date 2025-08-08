#[[
  Ignore compilation warnings for the entire 3rdparty directory,
  but keep all the other compilation options.
]]
get_directory_property(THIRD_PARTY_COMPILE_OPTIONS_ COMPILE_OPTIONS)
list(APPEND THIRD_PARTY_COMPILE_OPTIONS_ "-w")
set_directory_properties(PROPERTIES COMPILE_OPTIONS
                                    "${THIRD_PARTY_COMPILE_OPTIONS_}")

add_subdirectory(googletest EXCLUDE_FROM_ALL)

# add the custom extensions for unit tests
target_include_directories(
    gtest_main
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/googletest/extensions>")

# configure printf options
set(BUILD_STATIC_LIBRARY
    ON
    CACHE BOOL "Build the library as static rather than shared" FORCE)

set(ALIAS_STANDARD_FUNCTION_NAMES
    OFF
    CACHE
        BOOL
        "Alias the standard library function names (printf, sprintf etc.) to the library's functions"
        FORCE)

set(SUPPORT_DECIMAL_SPECIFIERS
    OFF
    CACHE
        BOOL
        "Support decimal notation floating-point conversion specifiers (%f,%F)"
        FORCE)

set(SUPPORT_EXPONENTIAL_SPECIFIERS
    OFF
    CACHE
        BOOL
        "Support exponential floating point format conversion specifiers (%e,%E,%g,%G)"
        FORCE)

add_subdirectory(printf EXCLUDE_FROM_ALL)

add_subdirectory(freeRtos EXCLUDE_FROM_ALL)


set(LWIP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lwip")
set(LWIP_INCLUDE_DIRS
    "${LWIP_DIR}/src/include"
    "${LWIP_DIR}/contrib"
    "${OPENBSW_APP_DIR}/lwipConfiguration/include"
    "${OPENBSW_PLATFORM_DIR}/lwipSysArch/include"
    "${OPENBSW_DIR}/libs/bsw/platform/include")
include(lwip/src/Filelists.cmake)

target_include_directories(
    lwipcore PUBLIC "${LWIP_DIR}/src/include"
                    "${OPENBSW_APP_DIR}/application/include")

if (NOT BUILD_UNIT_TESTS)
target_link_libraries(lwipcore PUBLIC lwipConfiguration)
endif ()
add_library(etl INTERFACE)

target_include_directories(etl INTERFACE etl/include etl_configuration etl_addons/include)
