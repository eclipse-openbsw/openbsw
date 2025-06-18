add_library(
    lifecycle
    src/lifecycle/AsyncLifecycleComponent.cpp
    src/lifecycle/LifecycleComponent.cpp
    src/lifecycle/LifecycleLogger.cpp
    src/lifecycle/LifecycleManager.cpp
    src/lifecycle/LifecycleManagerForwarder.cpp
    src/lifecycle/SimpleLifecycleComponent.cpp
    src/lifecycle/SingleContextLifecycleComponent.cpp)

target_include_directories(lifecycle PUBLIC include)

target_link_libraries(lifecycle PUBLIC async util etl)

if (BUILD_UNIT_TESTS)

    add_library(lifecycleMock INTERFACE)

    target_include_directories(lifecycleMock INTERFACE mock/gmock/include)

    target_link_libraries(lifecycleMock INTERFACE gmock lifecycle)

endif ()
