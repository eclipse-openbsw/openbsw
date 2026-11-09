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
    ${_GEN_INC}/middleware/shm/AllocatorsDefinitions.h
    ${_GEN_INC}/middleware/shm/QueueDefinitions.h
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

if (PLATFORM_SUPPORT_SOMEIP_BRIDGE_SAMPLE)
    # The SOME/IP bridge sample's service comes from the FLYNC model. Its
    # deployment is merged into deployment.yaml, so Foo and the sample share one
    # memory layout and one set of cluster connections.
    set(_FLYNC_MODEL ${CMAKE_SOURCE_DIR}/flync-configuration/flync)
    set(_FLYNC_DIR ${_GEN_DIR}/flync)
    set(_MERGE_SCRIPT ${_GEN_INPUT}/merge_deployment.py)
    file(GLOB_RECURSE _FLYNC_MODEL_FILES CONFIGURE_DEPENDS
         ${_FLYNC_MODEL}/*.flync.yaml)
    file(GLOB _FLYNC_GENERATOR CONFIGURE_DEPENDS
         ${_GEN_INPUT}/flync2middleware/*.py)

    list(
        APPEND
        _GENERATED_HEADERS
        ${_GEN_INC}/middleware/ClusterCluster2.h
        ${_GEN_INC}/middleware/ClusterCluster3.h
        ${_GEN_INC}/middleware/ClusterConnectionsCluster2Cluster3.h
        ${_GEN_INC}/middleware/ClusterConnectionsCluster3Cluster2.h
        ${_GEN_INC}/ets/EtsCodecs.h
        ${_GEN_INC}/ets/EtsCommon.h
        ${_GEN_INC}/ets/EtsProxy.h
        ${_GEN_INC}/ets/EtsSkeleton.h)

    list(
        APPEND
        GENERATED_SRCS
        ${_GEN_SRC}/ClusterCluster2.cpp
        ${_GEN_SRC}/ClusterCluster3.cpp
        ${_GEN_SRC}/ets/EtsCommon.cpp
        ${_GEN_SRC}/ets/EtsProxy.cpp
        ${_GEN_SRC}/ets/EtsSkeleton.cpp)

    add_custom_command(
        OUTPUT ${_GENERATED_HEADERS} ${GENERATED_SRCS}
        COMMAND
            ${Python3_EXECUTABLE} ${_GEN_SCRIPT} --input ${_GEN_INPUT} --output
            ${_FLYNC_DIR} --flync ${_FLYNC_MODEL} --emit-deployment
            ${_FLYNC_DIR}/deployment.yaml
        COMMAND ${Python3_EXECUTABLE} ${_MERGE_SCRIPT} ${_DEPLOYMENT_YAML}
                ${_FLYNC_DIR}/deployment.yaml ${_GEN_DIR}/deployment.merged.yaml
        COMMAND
            ${Python3_EXECUTABLE} ${_GEN_SCRIPT} --input ${_GEN_INPUT} --output
            ${_GEN_DIR} --deployment-yaml ${_GEN_DIR}/deployment.merged.yaml
        COMMAND
            ${CMAKE_COMMAND} -E copy
            ${_FLYNC_DIR}/include/generated_code/ets/EtsCodecs.h ${_GEN_INC}/ets
        DEPENDS ${_GEN_SCRIPT}
                ${_MERGE_SCRIPT}
                ${_FLYNC_GENERATOR}
                ${_FLYNC_MODEL_FILES}
                ${_DEPLOYMENT_YAML}
                ${_SCHEMA_YAML}
                ${_JINJA_TEMPLATES}
        COMMENT
            "Generating middleware C++ code from deployment.yaml and the FLYNC model"
        VERBATIM)
else ()
    add_custom_command(
        OUTPUT ${_GENERATED_HEADERS} ${GENERATED_SRCS}
        COMMAND ${Python3_EXECUTABLE} ${_GEN_SCRIPT} --input ${_GEN_INPUT}
                --output ${_GEN_DIR} --deployment-yaml ${_DEPLOYMENT_YAML}
        DEPENDS ${_GEN_SCRIPT} ${_DEPLOYMENT_YAML} ${_SCHEMA_YAML}
                ${_JINJA_TEMPLATES}
        COMMENT "Generating middleware C++ code from deployment.yaml"
        VERBATIM)
endif ()

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
