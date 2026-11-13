find_package(Python3 REQUIRED Interpreter)

set(_ROUTING_TOOLS_DIR ${CMAKE_SOURCE_DIR}/tools)
set(_BLOB_GENERATOR ${_ROUTING_TOOLS_DIR}/blob/generate_routing.py)
set(_BLOB_TEST_INPUT ${CMAKE_CURRENT_SOURCE_DIR}/routing.jsonl)
set(_BLOB_TEST_GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)

file(GLOB_RECURSE _BLOB_GENERATOR_MODULES CONFIGURE_DEPENDS
     ${_ROUTING_TOOLS_DIR}/blob/*.py)

set(_BLOB_TEST_GENERATED_HEADERS
    ${_BLOB_TEST_GENERATED_DIR}/blob/ConfigType.h
    ${_BLOB_TEST_GENERATED_DIR}/blob/configuration.h
    ${_BLOB_TEST_GENERATED_DIR}/routing/channelId.h
    ${_BLOB_TEST_GENERATED_DIR}/routing/constants.h)

add_custom_command(
    OUTPUT ${_BLOB_TEST_GENERATED_HEADERS}
    COMMAND
        ${CMAKE_COMMAND} -E env "PYTHONPATH=${_ROUTING_TOOLS_DIR}"
        ${Python3_EXECUTABLE} ${_BLOB_GENERATOR} ${_BLOB_TEST_INPUT}
        ${_BLOB_TEST_GENERATED_DIR}/blob ${_BLOB_TEST_GENERATED_DIR}/routing
    DEPENDS ${_BLOB_TEST_INPUT} ${_BLOB_GENERATOR_MODULES}
    COMMENT "Generating blob test configuration headers"
    VERBATIM)

add_custom_target(blobTestConfigurationGenerated
                  DEPENDS ${_BLOB_TEST_GENERATED_HEADERS})

add_dependencies(blob blobTestConfigurationGenerated)

target_include_directories(blobConfiguration BEFORE
                           INTERFACE ${_BLOB_TEST_GENERATED_DIR})

target_include_directories(blobConfiguration INTERFACE include)

add_executable(blobTest src/BlobTest.cpp)

add_dependencies(blobTest blobTestConfigurationGenerated)

target_link_libraries(blobTest PRIVATE blob blobConfiguration gmock_main)

gtest_discover_tests(blobTest PROPERTIES LABELS "blobTest")
