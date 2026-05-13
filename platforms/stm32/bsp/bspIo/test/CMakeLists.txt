# Copyright 2024 Contributors to the Eclipse Foundation
#
# SPDX-License-Identifier: EPL-2.0

add_executable(bspIoTest src/io/GpioTest.cpp)

target_include_directories(bspIoTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)

target_link_libraries(bspIoTest PRIVATE gtest_main)

gtest_discover_tests(bspIoTest PROPERTIES LABELS "bspIoTest")
