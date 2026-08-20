find_package(Python3 REQUIRED COMPONENTS Interpreter)

# Both example targets use the same deployment model and generator templates.
set(_MIDDLEWARE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..)
set(_GEN_SCRIPT ${_MIDDLEWARE_ROOT}/tools/cpp_generator/jinja2cpp.py)
set(_GEN_INPUT ${_MIDDLEWARE_ROOT}/tools/cpp_generator)
set(_DEPLOYMENT_YAML ${CMAKE_CURRENT_SOURCE_DIR}/deployment.yaml)
set(_SCHEMA_YAML ${_GEN_INPUT}/templates/schemas/deployment.yaml)

file(GLOB_RECURSE _JINJA_TEMPLATES CONFIGURE_DEPENDS
     ${_GEN_INPUT}/templates/jinja/*.jinja)

# Release example: generate the complete middleware interface and transport
# code.
set(_RELEASE_GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/release)
set(_RELEASE_GEN_INC ${_RELEASE_GEN_DIR}/include/generated_code)
set(_RELEASE_GEN_SRC ${_RELEASE_GEN_DIR}/src/generated_code)
set(_RELEASE_GENERATED_HEADERS
    ${_RELEASE_GEN_INC}/middleware/ClusterId.h
    ${_RELEASE_GEN_INC}/middleware/ClusterCore0.h
    ${_RELEASE_GEN_INC}/middleware/ClusterCore1.h
    ${_RELEASE_GEN_INC}/middleware/ClusterConnectionsCore0Core1.h
    ${_RELEASE_GEN_INC}/middleware/ClusterConnectionsCore1Core0.h
    ${_RELEASE_GEN_INC}/middleware/shm/Config.h
    ${_RELEASE_GEN_INC}/shm/AllocatorsDefinitions.h
    ${_RELEASE_GEN_INC}/shm/QueueDefinitions.h
    ${_RELEASE_GEN_INC}/features/communication/dummy_serviceCommon.h
    ${_RELEASE_GEN_INC}/features/communication/dummy_serviceProxy.h
    ${_RELEASE_GEN_INC}/features/communication/dummy_serviceSkeleton.h)
set(_RELEASE_GENERATED_SOURCES
    ${_RELEASE_GEN_SRC}/AllocatorSelectorDefinitions.cpp
    ${_RELEASE_GEN_SRC}/ClusterCore0.cpp
    ${_RELEASE_GEN_SRC}/ClusterCore1.cpp
    ${_RELEASE_GEN_SRC}/features/communication/dummy_serviceCommon.cpp
    ${_RELEASE_GEN_SRC}/features/communication/dummy_serviceProxy.cpp
    ${_RELEASE_GEN_SRC}/features/communication/dummy_serviceSkeleton.cpp
    ${_RELEASE_GEN_SRC}/shm/Config.cpp)

add_custom_command(
    # The generator cleans its output tree, so the stamp target explicitly
    # serializes generation before compilation starts.
    OUTPUT ${_RELEASE_GEN_DIR}/.generated
    COMMAND ${Python3_EXECUTABLE} ${_GEN_SCRIPT} --input ${_GEN_INPUT} --output
            ${_RELEASE_GEN_DIR} --deployment-yaml ${_DEPLOYMENT_YAML}
    COMMAND ${CMAKE_COMMAND} -E touch ${_RELEASE_GEN_DIR}/.generated
    DEPENDS ${_GEN_SCRIPT} ${_DEPLOYMENT_YAML} ${_SCHEMA_YAML}
            ${_JINJA_TEMPLATES}
    COMMENT "Generating release middleware documentation example code"
    VERBATIM)
add_custom_target(middlewareDocExamplesReleaseCodegen
                  DEPENDS ${_RELEASE_GEN_DIR}/.generated)
set_source_files_properties(
    ${_RELEASE_GENERATED_HEADERS} ${_RELEASE_GENERATED_SOURCES}
    PROPERTIES GENERATED TRUE)

add_library(middlewareDocExamplesRelease STATIC ${_RELEASE_GENERATED_SOURCES})
target_include_directories(
    middlewareDocExamplesRelease
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/release ${_RELEASE_GEN_INC}
            ${_MIDDLEWARE_ROOT}/test/include)
target_link_libraries(middlewareDocExamplesRelease PRIVATE middleware)
add_dependencies(middlewareDocExamplesRelease
                 middlewareDocExamplesReleaseCodegen)

# Test example: generate only the service interfaces and mock-backed sources.
set(_TEST_GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/test)
set(_TEST_GEN_INC ${_TEST_GEN_DIR}/include/generated_code)
set(_TEST_GEN_SRC ${_TEST_GEN_DIR}/src/generated_code)
set(_TEST_GENERATED_HEADERS
    ${_TEST_GEN_INC}/middleware/ClusterId.h
    ${_TEST_GEN_INC}/shm/AllocatorsDefinitions.h
    ${_TEST_GEN_INC}/features/communication/dummy_serviceCommon.h
    ${_TEST_GEN_INC}/features/communication/dummy_serviceProxy.h
    ${_TEST_GEN_INC}/features/communication/dummy_serviceProxyMock.h
    ${_TEST_GEN_INC}/features/communication/dummy_serviceSkeleton.h
    ${_TEST_GEN_INC}/features/communication/dummy_serviceSkeletonMock.h)
set(_TEST_GENERATED_SOURCES
    ${_TEST_GEN_SRC}/features/communication/dummy_serviceProxy.cpp
    ${_TEST_GEN_SRC}/features/communication/dummy_serviceSkeleton.cpp)

add_custom_command(
    OUTPUT ${_TEST_GEN_DIR}/.generated
    COMMAND
        ${Python3_EXECUTABLE} ${_GEN_SCRIPT} --input ${_GEN_INPUT} --output
        ${_TEST_GEN_DIR} --deployment-yaml ${_DEPLOYMENT_YAML} --generation-mode
        mock
    COMMAND ${CMAKE_COMMAND} -E touch ${_TEST_GEN_DIR}/.generated
    DEPENDS ${_GEN_SCRIPT} ${_DEPLOYMENT_YAML} ${_SCHEMA_YAML}
            ${_JINJA_TEMPLATES}
    COMMENT "Generating mock middleware documentation example code"
    VERBATIM)
add_custom_target(middlewareDocExamplesTestCodegen
                  DEPENDS ${_TEST_GEN_DIR}/.generated)
set_source_files_properties(
    ${_TEST_GENERATED_HEADERS} ${_TEST_GENERATED_SOURCES} PROPERTIES GENERATED
                                                                     TRUE)

add_executable(
    middlewareDocExamplesTest
    # Reuse the middleware test platform seams required by the core library.
    test/proxy_mock_unit_test_example.cpp
    test/skeleton_mock_unit_test_example.cpp
    ${_MIDDLEWARE_ROOT}/test/src/memory/mock/AllocatorMock.cpp
    ${_MIDDLEWARE_ROOT}/test/src/logger/mock/LoggerMock.cpp
    ${_MIDDLEWARE_ROOT}/test/src/os/OsDefinitions.cpp
    ${_TEST_GENERATED_SOURCES})
target_include_directories(
    middlewareDocExamplesTest
    PRIVATE ${_TEST_GEN_INC}
            ${_MIDDLEWARE_ROOT}/test/src
            ${_MIDDLEWARE_ROOT}/test/include
            ${_MIDDLEWARE_ROOT}/test/mock/include
            ${CMAKE_CURRENT_SOURCE_DIR}/release)
target_link_libraries(middlewareDocExamplesTest PRIVATE middleware gmock_main)
add_dependencies(middlewareDocExamplesTest middlewareDocExamplesTestCodegen)
