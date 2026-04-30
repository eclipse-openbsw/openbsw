# Platform configuration should be inherited from parent CMakeLists.txt
if (NOT DEFINED OPENBSW_TARGET)
    string(TOLOWER "${BUILD_TARGET_PLATFORM}" OPENBSW_TARGET)
endif ()

add_subdirectory(platforms/${OPENBSW_TARGET})

# Configure async and Rtos libs depending on target rtos.
if (BUILD_TARGET_RTOS STREQUAL "FREERTOS")
    add_library(asyncPlatform ALIAS asyncFreeRtos)
    add_library(osRtos ALIAS freeRtos)
    add_library(asyncRtosImpl ALIAS asyncFreeRtosImpl)
elseif (BUILD_TARGET_RTOS STREQUAL "THREADX")
    add_library(asyncPlatform ALIAS asyncThreadX)
    add_library(osRtos ALIAS threadX)
    add_library(asyncRtosImpl ALIAS asyncThreadXImpl)
endif ()

add_subdirectory(application)
add_subdirectory(asyncBinding)
add_subdirectory(asyncCoreConfiguration)
add_subdirectory(consoleCommands)

# Empty dummy lib for integration test
add_library(commonImpl INTERFACE)
