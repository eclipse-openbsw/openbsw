# Platform configuration should be inherited from parent CMakeLists.txt
if (NOT DEFINED OPENBSW_TARGET)
    string(TOLOWER "${BUILD_TARGET_PLATFORM}" OPENBSW_TARGET)
endif ()

add_subdirectory(platforms/${OPENBSW_TARGET})

# Configure async and Rtos libs depending on target rtos.
if (BUILD_TARGET_RTOS STREQUAL "FREERTOS")
    add_library(asyncPlatform ALIAS asyncFreeRtos)
    add_library(osFreeRtos ALIAS freeRtos)
elseif (BUILD_TARGET_RTOS STREQUAL "THREADX")
    add_library(asyncPlatform ALIAS asyncThreadX)
    add_library(osThreadX ALIAS threadX)
endif ()

# Injections
add_subdirectory(application)
add_subdirectory(asyncBinding)
add_subdirectory(asyncCoreConfiguration)
add_subdirectory(configuration)
add_subdirectory(consoleCommands)
add_subdirectory(udsConfiguration)
add_subdirectory(transportConfiguration)
add_subdirectory(lwipConfiguration)
add_subdirectory(safety)
