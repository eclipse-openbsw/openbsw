add_library(asyncThreadX INTERFACE)

target_include_directories(asyncThreadX INTERFACE include)

target_link_libraries(asyncThreadX INTERFACE asyncImpl asyncThreadXImpl
                                             bspInterrupts timer)

add_library(asyncThreadXImpl src/async/Async.cpp src/async/FutureSupport.cpp
                             src/async/Hook.cpp src/async/Types.cpp)

target_include_directories(asyncThreadXImpl PRIVATE include)

target_link_libraries(
    asyncThreadXImpl
    PUBLIC bsp threadX
    PRIVATE asyncBinding asyncCoreConfiguration etl util)

if (BUILD_UNIT_TESTS)
    # These compile definitions apply to all unit test
    target_compile_definitions(
        asyncThreadX INTERFACE ASYNC_LOOPER_DISABLE
                               ASYNC_TIMEOUTMANAGER2_DISABLE)

    # add_library(asyncBinding INTERFACE)

    # target_include_directories(asyncBinding INTERFACE test/include)

    # add_library(asyncCoreConfiguration INTERFACE)

    # target_include_directories(asyncCoreConfiguration INTERFACE test/include)

endif ()
