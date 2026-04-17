# Copyright 2024 Contributors to the Eclipse Foundation
#
# SPDX-License-Identifier: EPL-2.0

add_library(safeLifecycle src/SafetyManager.cpp)

target_include_directories(safeLifecycle PUBLIC include)

target_link_libraries(safeLifecycle PRIVATE safeSupervisor safeUtils safeBspMcuWatchdog)
