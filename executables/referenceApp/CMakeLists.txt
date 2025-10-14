# Platform configuration should be inherited from parent CMakeLists.txt
if (NOT DEFINED OPENBSW_TARGET)
    string(TOLOWER "${BUILD_TARGET_PLATFORM}" OPENBSW_TARGET)
endif ()

add_subdirectory(platforms/${OPENBSW_TARGET})

# Configure async and freeRtos libs depending on build type.
add_library(asyncPlatform ALIAS asyncFreeRtos)
add_library(osFreeRtos ALIAS freeRtos)

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
