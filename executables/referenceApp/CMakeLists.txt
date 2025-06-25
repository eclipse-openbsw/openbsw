option(PLATFORM_SUPPORT_CAN "Turn CAN support on or off" OFF)
option(PLATFORM_SUPPORT_ETHERNET "Turn ethernet support on or off" OFF)
option(PLATFORM_SUPPORT_IO "Turn IO support on or off" OFF)
option(PLATFORM_SUPPORT_UDS "Turn UDS support on or off" OFF)
option(PLATFORM_SUPPORT_STORAGE "Turn persistent storage on or off" OFF)

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
