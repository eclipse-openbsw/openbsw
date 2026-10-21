# *******************************************************************************
# Copyright (c) 2026 BMW AG
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

add_library(
    middlewareTestSupport STATIC
    src/logger/mock/LoggerMock.cpp src/memory/mock/AllocatorMock.cpp
    src/os/OsDefinitions.cpp src/time/mock/SystemTimerProviderMock.cpp)

target_include_directories(middlewareTestSupport
                           PUBLIC src ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(middlewareTestSupport
                      PUBLIC middleware middlewareInterfaces etl gmock)

# Defines the platform concurrency hooks; link only into tests that need them.
add_library(middlewareConcurrencyMock STATIC
            src/concurrency/mock/ConcurrencyMock.cpp)
target_include_directories(middlewareConcurrencyMock PUBLIC src)
target_link_libraries(middlewareConcurrencyMock PUBLIC middlewareInterfaces
                                                       gmock)

# The middleware sources include lock_types.h (via LockStrategies.h).
target_include_directories(middleware
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(MIDDLEWARE_TEST_TARGETS)

function (add_middleware_unit_test TARGET_NAME)
    add_executable(${TARGET_NAME} ${ARGN})
    target_link_libraries(${TARGET_NAME}
                          PRIVATE middleware middlewareTestSupport gtest_main)
    gtest_discover_tests(${TARGET_NAME} PROPERTIES LABELS "middlewareTest")
    set(MIDDLEWARE_TEST_TARGETS
        ${MIDDLEWARE_TEST_TARGETS} ${TARGET_NAME}
        PARENT_SCOPE)
endfunction ()

add_middleware_unit_test(
    middlewareProxySkeletonBaseTest
    src/core/ProxyAttributesTest.cpp
    src/core/ProxyBaseTest.cpp
    src/core/SkeletonAttributeTest.cpp
    src/core/SkeletonBaseTest.cpp
    src/core/TypesTest.cpp)

add_middleware_unit_test(
    middlewareRequestResponseTest src/core/ResponseBufferTest.cpp
    src/rpc/ProxyFireAndForgetMethodTest.cpp src/rpc/ProxyMethodTest.cpp)

add_middleware_unit_test(middlewareInstanceDatabaseTest
                         src/core/DbManipulatorTest.cpp)

add_middleware_unit_test(
    middlewareClusterConnectionTest src/core/ClusterConfigurationTest.cpp
    src/core/ConnectionTest.cpp)

add_middleware_unit_test(
    middlewareMessageManagementTest src/core/MessagePayloadBuilderTest.cpp
    src/core/MessageTest.cpp)

add_middleware_unit_test(middlewarePduTest src/core/MiddlewarePduTest.cpp)
target_link_libraries(middlewarePduTest PRIVATE middlewareConcurrencyMock)

add_middleware_unit_test(middlewareQueueTest src/queue/QueueTest.cpp)

add_middleware_unit_test(
    middlewareMemoryPoolManagementTest src/memory/AggregatorTest.cpp
    src/memory/AllocatorBaseTest.cpp src/memory/PoolsTest.cpp)

add_middleware_unit_test(middlewareLoggingApiTest src/core/LoggerApi.cpp)

add_custom_target(middlewareTest DEPENDS ${MIDDLEWARE_TEST_TARGETS})

if (UNIX)
    add_subdirectory(../simulation ${CMAKE_CURRENT_BINARY_DIR}/simulation)
    add_dependencies(middlewareTest middlewareSimulationFooMethodTest)
endif ()
