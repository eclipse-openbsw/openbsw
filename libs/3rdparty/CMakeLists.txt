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

add_library(etl INTERFACE)

target_include_directories(etl INTERFACE etl/include)
