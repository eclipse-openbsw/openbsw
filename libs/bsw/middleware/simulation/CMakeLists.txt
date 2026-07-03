if (NOT UNIX)
    return()
endif ()

find_package(Python3 REQUIRED COMPONENTS Interpreter)
find_package(Threads REQUIRED)

# -----------------------------------------------------------------------
# Platform integration sources (concurrency / logger / OS / time stubs)
# -----------------------------------------------------------------------
set(PLATFORM_INTEGRATION_SRCS
    platform_integration/logger/src/LoggerImpl.cpp
    platform_integration/os/src/OsDefinitions.cpp
    platform_integration/time/src/SystemTimeProvider.cpp)

# -----------------------------------------------------------------------
# Generated code sources (produced by jinja2cpp.py from
# model/deployment-test.yaml)
# -----------------------------------------------------------------------
set(GENERATED_SRCS
    src/generated_code/AllocatorSelectorDefinitions.cpp
    src/generated_code/ClusterCore0.cpp
    src/generated_code/ClusterCore1.cpp
    src/generated_code/shm/Config.cpp
    src/generated_code/org/test/foo/FooCommon.cpp
    src/generated_code/org/test/foo/FooProxy.cpp
    src/generated_code/org/test/foo/FooSkeleton.cpp)

# -----------------------------------------------------------------------
# Simulation application
# -----------------------------------------------------------------------
add_executable(
    middlewareSimulation
    ${PLATFORM_INTEGRATION_SRCS}
    ${GENERATED_SRCS}
    src/Logger.cpp
    src/ShmWrapper.cpp
    src/MainCore0.cpp
    src/MainCore1.cpp
    src/Sample.cpp)

target_include_directories(
    middlewareSimulation
    PRIVATE # Simulation-specific headers (logger.h, wrappers, etc.)
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            # Generated headers (middleware/cluster_*.h, shm/*.h,
            # org/test/foo/*.h, etc.)
            ${CMAKE_CURRENT_SOURCE_DIR}/include/generated_code
            # Custom allocator ("allocator.h" is included bare by generated
            # code)
            ${CMAKE_CURRENT_SOURCE_DIR}/include/allocator
            # Platform concurrency stubs
            ${CMAKE_CURRENT_SOURCE_DIR}/platform_integration/concurrency/include
)

target_link_libraries(middlewareSimulation PRIVATE middleware Threads::Threads
                                                   rt)

target_include_directories(
    middlewareSimulation
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/platform_integration/concurrency/include
)

# Expose the platform concurrency stubs to the middleware library itself
# (middleware/interfaces/include/middleware/concurrency/LockStrategies.h
# includes <middleware/concurrency/lock_types.h> which lives in
# platform_integration).
target_include_directories(
    middleware
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/platform_integration/concurrency/include)

# Optionally build with process-based concurrency instead of threads: cmake
# -DSIMULATION_USE_PROCESSES=ON ...
option(SIMULATION_USE_PROCESSES
       "Run each simulated core in a separate OS process (default: threads)"
       OFF)
if (SIMULATION_USE_PROCESSES)
    target_compile_definitions(middlewareSimulation
                               PRIVATE SIMULATION_USE_PROCESSES)
endif ()
