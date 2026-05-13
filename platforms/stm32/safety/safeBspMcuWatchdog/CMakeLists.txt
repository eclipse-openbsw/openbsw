# Copyright 2024 Contributors to the Eclipse Foundation
#
# SPDX-License-Identifier: EPL-2.0

add_library(safeBspMcuWatchdog src/watchdog/Watchdog.cpp)
target_include_directories(safeBspMcuWatchdog PUBLIC include)
target_link_libraries(safeBspMcuWatchdog PUBLIC bspMcu PRIVATE platform)
