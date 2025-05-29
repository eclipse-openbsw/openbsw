add_executable(
    asyncImplTest
    src/async/EventDispatcherTest.cpp
    src/async/EventPolicyTest.cpp
    src/async/QueueNodeTest.cpp
    src/async/QueueTest.cpp
    src/async/RunnableExecutorTest.cpp)

target_include_directories(asyncImplTest PRIVATE ../include)

target_link_libraries(asyncImplTest PRIVATE asyncImpl asyncImplMock gmock_main etl)

gtest_discover_tests(asyncImplTest PROPERTIES LABELS "asyncImplTest")
