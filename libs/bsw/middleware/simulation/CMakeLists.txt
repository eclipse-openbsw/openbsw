if (NOT UNIX)
    return()
endif ()

find_package(Python3 REQUIRED COMPONENTS Interpreter)
find_package(Threads REQUIRED)

# -----------------------------------------------------------------------
# Code generation via jinja2cpp.py
# -----------------------------------------------------------------------
set(_GEN_SCRIPT
    ${CMAKE_SOURCE_DIR}/libs/bsw/middleware/tools/cpp_generator/jinja2cpp.py)
set(_GEN_INPUT ${CMAKE_SOURCE_DIR}/libs/bsw/middleware/tools/cpp_generator)
set(_DEPLOYMENT_YAML ${CMAKE_CURRENT_SOURCE_DIR}/model/deployment-test.yaml)
set(_SCHEMA_YAML ${_GEN_INPUT}/templates/schemas/deployment.yaml)

file(GLOB_RECURSE _JINJA_TEMPLATES CONFIGURE_DEPENDS
     ${_GEN_INPUT}/templates/jinja/*.jinja)

set(_GEN_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(_GEN_INC ${_GEN_DIR}/include/generated_code)
set(_GEN_SRC ${_GEN_DIR}/src/generated_code)

set(_GENERATED_HEADERS
    ${_GEN_INC}/middleware/ClusterId.h
    ${_GEN_INC}/middleware/ClusterCluster0.h
    ${_GEN_INC}/middleware/ClusterCluster1.h
    ${_GEN_INC}/middleware/ClusterConnectionsCluster0Cluster1.h
    ${_GEN_INC}/middleware/ClusterConnectionsCluster1Cluster0.h
    ${_GEN_INC}/middleware/shm/Config.h
    ${_GEN_INC}/shm/AllocatorsDefinitions.h
    ${_GEN_INC}/shm/QueueDefinitions.h
    ${_GEN_INC}/org/test/foo/FooCommon.h
    ${_GEN_INC}/org/test/foo/FooProxy.h
    ${_GEN_INC}/org/test/foo/FooSkeleton.h)

set(GENERATED_SRCS
    ${_GEN_SRC}/AllocatorSelectorDefinitions.cpp
    ${_GEN_SRC}/ClusterCluster0.cpp
    ${_GEN_SRC}/ClusterCluster1.cpp
    ${_GEN_SRC}/shm/Config.cpp
    ${_GEN_SRC}/org/test/foo/FooCommon.cpp
    ${_GEN_SRC}/org/test/foo/FooProxy.cpp
    ${_GEN_SRC}/org/test/foo/FooSkeleton.cpp)

add_custom_command(
    OUTPUT ${_GENERATED_HEADERS} ${GENERATED_SRCS}
    COMMAND ${Python3_EXECUTABLE} ${_GEN_SCRIPT} --input ${_GEN_INPUT} --output
            ${_GEN_DIR} --deployment-yaml ${_DEPLOYMENT_YAML}
    DEPENDS ${_GEN_SCRIPT} ${_DEPLOYMENT_YAML} ${_SCHEMA_YAML}
            ${_JINJA_TEMPLATES}
    COMMENT "Generating middleware C++ code from deployment-test.yaml"
    VERBATIM)

# -----------------------------------------------------------------------
# Platform integration sources (concurrency / logger / OS / time stubs)
# -----------------------------------------------------------------------
set(PLATFORM_INTEGRATION_SRCS
    platform_integration/logger/src/LoggerImpl.cpp
    platform_integration/os/src/OsDefinitions.cpp
    platform_integration/time/src/SystemTimeProvider.cpp)

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
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${_GEN_DIR}/include/generated_code
            ${CMAKE_CURRENT_SOURCE_DIR}/include/allocator
            ${CMAKE_CURRENT_SOURCE_DIR}/platform_integration/concurrency/include
)

target_link_libraries(middlewareSimulation PRIVATE middleware Threads::Threads
                                                   rt)

target_include_directories(
    middlewareSimulation
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/platform_integration/concurrency/include
)

target_include_directories(
    middleware
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/platform_integration/concurrency/include)

option(SIMULATION_USE_PROCESSES
       "Run each simulated core in a separate OS process (default: threads)"
       OFF)
if (SIMULATION_USE_PROCESSES)
    target_compile_definitions(middlewareSimulation
                               PRIVATE SIMULATION_USE_PROCESSES)
endif ()
