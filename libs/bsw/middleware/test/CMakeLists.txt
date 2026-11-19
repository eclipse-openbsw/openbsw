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
                           PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(MIDDLEWARE_TEST_TARGETS)

function (add_middleware_unit_test TARGET_NAME LIBRARY)
    add_executable(${TARGET_NAME} ${ARGN})
    target_link_libraries(${TARGET_NAME}
                          PRIVATE ${LIBRARY} middlewareTestSupport gtest_main)
    gtest_discover_tests(${TARGET_NAME} PROPERTIES LABELS "middlewareTest")
    set(MIDDLEWARE_TEST_TARGETS
        ${MIDDLEWARE_TEST_TARGETS} ${TARGET_NAME}
        PARENT_SCOPE)
endfunction ()

# Signal-based communication
add_middleware_unit_test(middlewareSigPduTest middlewareSig
                         src/sig/core/MiddlewarePduTest.cpp)
target_link_libraries(middlewareSigPduTest PRIVATE middlewareConcurrencyMock)

add_middleware_unit_test(
    middlewareSigPhysicalSignalConversionTest middlewareSig
    src/sig/core/PhysicalSignalConversionTest.cpp)

add_middleware_unit_test(middlewareSigSignalCodecTest middlewareSig
                         src/sig/core/SignalCodecTest.cpp)

add_middleware_unit_test(
    middlewareSigTransmissionTest
    middlewareSig
    src/sig/core/TimerTest.cpp
    src/sig/core/TransmissionModeManagerTest.cpp
    src/sig/core/TransmissionModeTest.cpp
    src/sig/core/TransmissionTest.cpp)

add_middleware_unit_test(
    middlewareSigPduBrokerTest middlewareSig src/sig/core/PduBrokerTest.cpp
    src/sig/core/PduSlotAccessTest.cpp)
target_link_libraries(middlewareSigPduBrokerTest
                      PRIVATE middlewareConcurrencyMock)

# Service-based communication
add_middleware_unit_test(
    middlewareSvcProxySkeletonBaseTest
    middlewareSvc
    src/svc/core/ProxyAttributesTest.cpp
    src/svc/core/ProxyBaseTest.cpp
    src/svc/core/SkeletonAttributeTest.cpp
    src/svc/core/SkeletonBaseTest.cpp
    src/svc/core/TypesTest.cpp)

add_middleware_unit_test(
    middlewareSvcRequestResponseTest
    middlewareSvc
    src/svc/core/ResponseBufferTest.cpp
    src/svc/rpc/ProxyFireAndForgetMethodTest.cpp
    src/svc/rpc/ProxyMethodTest.cpp)

add_middleware_unit_test(middlewareSvcInstanceDatabaseTest middlewareSvc
                         src/svc/core/DbManipulatorTest.cpp)

add_middleware_unit_test(
    middlewareSvcClusterConnectionTest middlewareSvc
    src/svc/core/ClusterConfigurationTest.cpp src/svc/core/ConnectionTest.cpp)

add_middleware_unit_test(
    middlewareSvcMessageManagementTest middlewareSvc
    src/svc/core/MessagePayloadBuilderTest.cpp src/svc/core/MessageTest.cpp)

add_middleware_unit_test(middlewareSvcLoggingApiTest middlewareSvc
                         src/svc/core/LoggerApi.cpp)

# Shared base
add_middleware_unit_test(middlewareQueueTest middleware src/queue/QueueTest.cpp)

add_middleware_unit_test(
    middlewareMemoryPoolManagementTest
    middleware
    src/memory/AggregatorTest.cpp
    src/memory/AllocatorBaseTest.cpp
    src/memory/PoolsTest.cpp)

add_custom_target(middlewareTest DEPENDS ${MIDDLEWARE_TEST_TARGETS})

if (UNIX)
    add_subdirectory(../simulation ${CMAKE_CURRENT_BINARY_DIR}/simulation)
    add_dependencies(middlewareTest middlewareSimulationFooMethodTest
                     middlewarePduWriterSchedulingTest)
endif ()
