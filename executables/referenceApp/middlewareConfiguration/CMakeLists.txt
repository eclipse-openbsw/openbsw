find_package(Python3 REQUIRED COMPONENTS Interpreter)

# -----------------------------------------------------------------------
# Code generation via jinja2cpp.py
# -----------------------------------------------------------------------
set(_GEN_SCRIPT
    ${CMAKE_SOURCE_DIR}/libs/bsw/middleware/tools/cpp_generator/jinja2cpp.py)
set(_GEN_INPUT ${CMAKE_SOURCE_DIR}/libs/bsw/middleware/tools/cpp_generator)
set(_DEPLOYMENT_YAML ${CMAKE_CURRENT_SOURCE_DIR}/model/deployment.yaml)
set(_SCHEMA_YAML ${_GEN_INPUT}/templates/schemas/deployment.yaml)

file(GLOB_RECURSE _JINJA_TEMPLATES CONFIGURE_DEPENDS
     ${_GEN_INPUT}/templates/jinja/*.jinja)
file(
    GLOB
    _GEN_PY_MODULES
    CONFIGURE_DEPENDS
    ${_GEN_INPUT}/*.py
    ${_GEN_INPUT}/filters/*.py
    ${_GEN_INPUT}/loaders/*.py)

set(_GEN_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(_GEN_INC ${_GEN_DIR}/include/generated_code)
set(_GEN_SRC ${_GEN_DIR}/src/generated_code)

set(_GENERATED_HEADERS
    ${_GEN_INC}/middleware/ClusterId.h
    ${_GEN_INC}/middleware/Cluster0Cluster.h
    ${_GEN_INC}/middleware/Cluster1Cluster.h
    ${_GEN_INC}/middleware/shm/Config.h
    ${_GEN_INC}/middleware/shm/AllocatorsDefinitions.h
    ${_GEN_INC}/middleware/shm/QueueDefinitions.h
    ${_GEN_INC}/org/test/foo/FooCommon.h
    ${_GEN_INC}/org/test/foo/FooProxy.h
    ${_GEN_INC}/org/test/foo/FooSkeleton.h
    ${_GEN_INC}/org/test/foo/FooTypes.h)

set(GENERATED_SRCS
    ${_GEN_SRC}/AllocatorSelectorDefinitions.cpp
    ${_GEN_SRC}/Cluster0Cluster.cpp
    ${_GEN_SRC}/Cluster1Cluster.cpp
    ${_GEN_SRC}/shm/Config.cpp
    ${_GEN_SRC}/org/test/foo/FooCommon.cpp
    ${_GEN_SRC}/org/test/foo/FooProxy.cpp
    ${_GEN_SRC}/org/test/foo/FooSkeleton.cpp)

add_custom_command(
    OUTPUT ${_GENERATED_HEADERS} ${GENERATED_SRCS}
    COMMAND ${Python3_EXECUTABLE} ${_GEN_SCRIPT} --input ${_GEN_INPUT} --output
            ${_GEN_DIR} --deployment-yaml ${_DEPLOYMENT_YAML}
    DEPENDS ${_GEN_SCRIPT}
            ${_GEN_PY_MODULES}
            ${_DEPLOYMENT_YAML}
            ${_SCHEMA_YAML}
            ${_JINJA_TEMPLATES}
    COMMENT "Generating middleware C++ code from deployment.yaml"
    VERBATIM)

set(PLATFORM_INTEGRATION_SRCS
    platform_integration/logger/src/LoggerImpl.cpp
    platform_integration/os/src/OsDefinitions.cpp
    platform_integration/time/src/SystemTimeProvider.cpp)

add_library(middlewareConfiguration
            ${GENERATED_SRCS} ${PLATFORM_INTEGRATION_SRCS} src/MemoryLayout.cpp)

target_include_directories(
    middlewareConfiguration
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
           ${_GEN_DIR}/include/generated_code
           ${CMAKE_CURRENT_SOURCE_DIR}/include/allocator)

target_include_directories(
    middlewareConfiguration
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/platform_integration/concurrency/include
            ${CMAKE_CURRENT_SOURCE_DIR}/../application/include)

target_include_directories(
    middleware
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/platform_integration/concurrency/include)

target_link_libraries(middlewareConfiguration PUBLIC asyncBinding middleware
                                                     logger)
