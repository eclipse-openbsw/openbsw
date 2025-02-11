add_library(asyncFreeRtos INTERFACE)

target_include_directories(asyncFreeRtos INTERFACE include)

target_link_libraries(
    asyncFreeRtos
    INTERFACE asyncImpl
              bsp
              bspInterrupts
              common
              osFreeRtos
              timer)

add_library(
    asyncFreeRtosImpl
    src/async/Async.cpp
    src/async/FutureSupport.cpp
    src/async/Hook.cpp
    src/async/Types.cpp)

target_include_directories(asyncFreeRtosImpl PRIVATE include)

target_link_libraries(asyncFreeRtosImpl PRIVATE asyncFreeRtos asyncBinding
                                                asyncCoreConfiguration estd)

# For FreeRTOSConfig.h, freertos_task_c_additions.h, Hook.h

add_library(freeRtosConfiguration INTERFACE)

target_include_directories(freeRtosConfiguration
                           INTERFACE freeRtosConfiguration)

target_link_libraries(
    freeRtosConfiguration INTERFACE asyncCoreConfiguration
                                    freeRtosCoreConfiguration platform)

if (BUILD_UNIT_TESTS)
    # These compile definitions apply to all unit test
    add_library(asyncBinding INTERFACE)

    target_include_directories(asyncBinding INTERFACE test/include)

    add_library(asyncCoreConfiguration INTERFACE)

    target_include_directories(asyncCoreConfiguration INTERFACE test/include)

    add_library(freeRtosCoreConfiguration INTERFACE)

    target_include_directories(freeRtosCoreConfiguration INTERFACE test/include)
endif ()
