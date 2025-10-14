add_executable(
    asyncFreeRtosTest
    src/async/AsyncTest.cpp
    src/async/FreeRtosAdapterTest.cpp
    src/async/FutureSupportTest.cpp
    src/async/LockTest.cpp
    src/async/ModifiableLockTest.cpp
    src/async/StaticRunnableTest.cpp
    src/async/TaskContextTest.cpp
    src/async/TaskInitializerTest.cpp
    mock/src/async/TickHookMock.cpp
    mock/src/async/ContextHookMock.cpp)

target_include_directories(asyncFreeRtosTest PRIVATE ../include mock/include)

target_link_libraries(
    asyncFreeRtosTest
    PRIVATE # asyncFreeRtos must be the first library to be targeted! This makes
            # sure, that the include path targeted by asyncFreeRtos are first in
            # the list of includes. This im required for the unit tests so that
            # the Types.h from asyncFreeRtos is used since asyncFreeRtos isa
            # asyncPlatform and thus needs itself to be the asyncPlatform for
            # its unit tests. All other modules use asyncMock as the
            # asyncPlatform for their tests.
            asyncFreeRtos
            async
            asyncImplMock
            asyncFreeRtosImpl
            bspMock
            freeRtosMock
            bspInterruptsMock
            gmock_main
            util)

add_executable(asyncFreeRtosMinimumStackTest
               src/async/TaskInitializerMinimumStackTest.cpp)

target_link_libraries(asyncFreeRtosMinimumStackTest PRIVATE asyncFreeRtos
                                                            gmock_main util)

target_compile_options(asyncFreeRtosTest PRIVATE -Wno-array-bounds)

gtest_discover_tests(asyncFreeRtosTest PROPERTIES LABELS "asyncFreeRtosTest")
gtest_discover_tests(asyncFreeRtosMinimumStackTest
                     PROPERTIES LABELS "asyncFreeRtosMinimumStackTest")
