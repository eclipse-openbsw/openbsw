add_executable(
    middlewareTest
    src/core/ClusterConfigurationTest.cpp
    src/core/ConnectionTest.cpp
    src/core/DbManipulatorTest.cpp
    src/core/LoggerApi.cpp
    src/core/MessagePayloadBuilderTest.cpp
    src/core/MessageTest.cpp
    src/core/ProxyAttributesTest.cpp
    src/core/ProxyBaseTest.cpp
    src/core/ResponseBufferTest.cpp
    src/core/SkeletonAttributeTest.cpp
    src/core/SkeletonBaseTest.cpp
    src/core/rpc/ProxyFireAndForgetMethodTest.cpp
    src/core/rpc/ProxyMethodTest.cpp
    src/memory/AggregatorTest.cpp
    src/memory/AllocatorBaseTest.cpp
    src/memory/PoolsTest.cpp
    src/mock/logger/mock/LoggerMock.cpp
    src/mock/memory/mock/AllocatorMock.cpp
    src/mock/os/OsDefinitions.cpp
    src/mock/time/mock/SystemTimerProviderMock.cpp
    src/queue/QueueTest.cpp)

target_include_directories(middlewareTest PUBLIC src src/mock
                                                 src/mock/concurrency)

target_link_libraries(middlewareTest PRIVATE middleware gmock gtest_main)

gtest_discover_tests(middlewareTest PROPERTIES LABELS "middlewareTest")
