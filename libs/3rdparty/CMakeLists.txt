if (BUILD_EXECUTABLE STREQUAL "unitTest")
    add_subdirectory(googletest)

    # add the custom extensions for unit tests
    target_include_directories(
        gtest_main
        PUBLIC
            "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/googletest/extensions>"
    )

endif ()

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

add_subdirectory(printf)

if (BUILD_TARGET_RTOS STREQUAL "FREERTOS")
    add_subdirectory(freeRtos EXCLUDE_FROM_ALL)
endif ()

if (BUILD_TARGET_RTOS STREQUAL "THREADX")
    add_subdirectory(threadx EXCLUDE_FROM_ALL)
endif ()

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

target_link_libraries(lwipcore PUBLIC lwipConfiguration)

set(GIT_DIR_LOOKUP_POLICY DISALLOW_LOOKING_ABOVE_CMAKE_SOURCE_DIR)
add_subdirectory(etl)
