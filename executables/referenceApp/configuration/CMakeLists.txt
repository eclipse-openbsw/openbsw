add_library(configuration common/src/busid/BusId.cpp)

target_include_directories(configuration PUBLIC include)

target_link_libraries(configuration PUBLIC common async)

find_package(Python3 REQUIRED Interpreter)

set(_ROUTING_TOOLS_DIR ${CMAKE_SOURCE_DIR}/tools)
set(_ROUTING_GEN_SCRIPT ${_ROUTING_TOOLS_DIR}/blob/generate_routing.py)
set(_ROUTING_INPUT ${CMAKE_CURRENT_SOURCE_DIR}/routing.jsonl)
set(_ROUTING_GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)

file(GLOB_RECURSE _ROUTING_GEN_MODULES CONFIGURE_DEPENDS
     ${_ROUTING_TOOLS_DIR}/blob/*.py)

set(_ROUTING_GENERATED_HEADERS
    ${_ROUTING_GEN_DIR}/blob/ConfigType.h
    ${_ROUTING_GEN_DIR}/blob/configuration.h
    ${_ROUTING_GEN_DIR}/routing/channelId.h
    ${_ROUTING_GEN_DIR}/routing/constants.h)

add_custom_command(
    OUTPUT ${_ROUTING_GENERATED_HEADERS}
    COMMAND
        ${CMAKE_COMMAND} -E env "PYTHONPATH=${_ROUTING_TOOLS_DIR}"
        ${Python3_EXECUTABLE} ${_ROUTING_GEN_SCRIPT} ${_ROUTING_INPUT}
        ${_ROUTING_GEN_DIR}/blob ${_ROUTING_GEN_DIR}/routing
    DEPENDS ${_ROUTING_INPUT} ${_ROUTING_GEN_MODULES}
    COMMENT "Generating reference application routing and blob headers"
    VERBATIM)

add_custom_target(routingConfigurationGenerated
                  DEPENDS ${_ROUTING_GENERATED_HEADERS})

add_dependencies(configuration routingConfigurationGenerated)
add_dependencies(blobConfiguration routingConfigurationGenerated)
add_dependencies(routingConfiguration routingConfigurationGenerated)

target_include_directories(configuration BEFORE PUBLIC ${_ROUTING_GEN_DIR})
target_include_directories(blobConfiguration BEFORE
                           INTERFACE ${_ROUTING_GEN_DIR})
target_include_directories(routingConfiguration BEFORE
                           INTERFACE ${_ROUTING_GEN_DIR})
