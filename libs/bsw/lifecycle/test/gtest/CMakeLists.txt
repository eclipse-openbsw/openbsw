add_executable(
    lifecycleTest
    src/lifecycle/AsyncLifecycleComponentTest.cpp
    src/lifecycle/LifecycleComponentTest.cpp
    src/lifecycle/LifecycleManagerForwarderTest.cpp
    src/lifecycle/LifecycleManagerInitializerTest.cpp
    src/lifecycle/LifecycleManagerTest.cpp
    src/lifecycle/SimpleLifecycleComponentTest.cpp
    src/lifecycle/SingleContextLifecycleComponentTest.cpp)

target_include_directories(lifecycle PUBLIC ../mock/gmock/include)

target_link_libraries(lifecycleTest PUBLIC lifecycle lifecycleMock
                                           asyncMockImpl gtest_main)

gtest_discover_tests(lifecycleTest PROPERTIES LABELS "lifecycleTest")
