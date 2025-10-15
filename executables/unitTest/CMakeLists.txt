add_library(asyncPlatform ALIAS asyncMock)
add_library(osFreeRtos ALIAS freeRtosMock)

add_subdirectory(bsp)
add_subdirectory(common)
add_subdirectory(configuration)
add_subdirectory(transportConfiguration)
add_subdirectory(udsConfiguration)
add_subdirectory(lwipConfiguration)
