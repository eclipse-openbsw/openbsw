add_library(bspConfiguration INTERFACE)

# Make EEPROM filepath configurable via CMake. Use a CACHE STRING (not option)
# so users can set an arbitrary path string via -D
set(BSP_EEPROM_FILEPATH
    "/tmp/openbsw_posix_eeprom.bin"
    CACHE STRING "Path to POSIX EEPROM file")

# Generate a header with the configured EEPROM file path into the build
# directory so consumers get the configured value first on the include path.
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/EepromConfiguration.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/EepromConfiguration.h @ONLY)

target_include_directories(
    bspConfiguration
    INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
