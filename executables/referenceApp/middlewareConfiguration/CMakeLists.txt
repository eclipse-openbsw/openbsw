if (NOT UNIX)
    return()
endif ()

set(GENERATED_SRCS
    src/generated_code/AllocatorSelectorDefinitions.cpp
    src/generated_code/ClusterCore0.cpp
    src/generated_code/ClusterCore1.cpp
    src/generated_code/shm/Config.cpp
    src/generated_code/org/test/foo/FooCommon.cpp
    src/generated_code/org/test/foo/FooProxy.cpp
    src/generated_code/org/test/foo/FooSkeleton.cpp)

set(PLATFORM_INTEGRATION_SRCS
    platform_integration/logger/src/LoggerImpl.cpp
    platform_integration/os/src/OsDefinitions.cpp
    platform_integration/time/src/SystemTimeProvider.cpp)

add_library(middlewareConfiguration
            ${GENERATED_SRCS} ${PLATFORM_INTEGRATION_SRCS} src/MemoryLayout.cpp)

target_include_directories(
    middlewareConfiguration
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
           ${CMAKE_CURRENT_SOURCE_DIR}/include/generated_code
           ${CMAKE_CURRENT_SOURCE_DIR}/include/allocator)

target_include_directories(
    middlewareConfiguration
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/platform_integration/concurrency/include
            ${CMAKE_CURRENT_SOURCE_DIR}/../application/include)

target_include_directories(
    middleware
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/platform_integration/concurrency/include)

target_link_libraries(middlewareConfiguration PUBLIC middleware logger)

