add_executable(
    middlewareTest
    src/core/ClusterConfigurationTest.cpp
    src/core/ConnectionTest.cpp
    src/core/DbManipulatorTest.cpp
    src/core/FutureDispatcherTest.cpp
    src/core/LoggerApi.cpp
    src/core/MessagePayloadBuilderTest.cpp
    src/core/MessageTest.cpp
    src/core/ProxyAttributesTest.cpp
    src/core/ProxyBaseTest.cpp
    src/core/ResponseBufferTest.cpp
    src/core/SkeletonAttributeTest.cpp
    src/core/SkeletonBaseTest.cpp
    src/logger/mock/LoggerMock.cpp
    src/memory/AllocatorBaseTest.cpp
    src/memory/mock/AllocatorMock.cpp
    src/memory/AggregatorTest.cpp
    src/memory/PoolsTest.cpp
    src/os/OsDefinitions.cpp
    src/queue/QueueTest.cpp
    src/time/mock/SystemTimerProviderMock.cpp)

target_include_directories(middlewareTest PUBLIC src)

target_include_directories(middleware
                           PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(middlewareTest PRIVATE middleware gmock gtest_main)

gtest_discover_tests(middlewareTest PROPERTIES LABELS "middlewareTest")
