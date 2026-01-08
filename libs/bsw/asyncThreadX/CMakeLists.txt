add_library(asyncThreadX INTERFACE)

target_include_directories(asyncThreadX INTERFACE include)

target_link_libraries(
    asyncThreadX
    INTERFACE asyncImpl
              bsp
              bspInterrupts
              common
              timer)

if (NOT BUILD_EXECUTABLE STREQUAL "unitTest")
    target_link_libraries(asyncThreadX INTERFACE threadX)
endif ()

add_library(asyncThreadXImpl src/async/Async.cpp src/async/FutureSupport.cpp
                             src/async/Hook.cpp src/async/Types.cpp)

target_include_directories(asyncThreadXImpl PRIVATE include)

target_link_libraries(
    asyncThreadXImpl
    PRIVATE asyncThreadX
            asyncBinding
            asyncCoreConfiguration
            etl
            util)

# For tx_user.h
add_library(threadXConfiguration INTERFACE)

target_include_directories(threadXConfiguration INTERFACE threadXConfiguration)

target_link_libraries(
    threadXConfiguration INTERFACE asyncCoreConfiguration platform
                                   threadXCoreConfiguration)

if (BUILD_EXECUTABLE STREQUAL "unitTest")
    # These compile definitions apply to all unit test
    target_compile_definitions(
        asyncThreadX INTERFACE ASYNC_LOOPER_DISABLE
                               ASYNC_TIMEOUTMANAGER2_DISABLE)
endif ()
