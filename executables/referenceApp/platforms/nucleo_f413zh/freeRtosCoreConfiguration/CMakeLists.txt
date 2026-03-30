# Copyright 2024 Contributors to the Eclipse Foundation
#
# SPDX-License-Identifier: EPL-2.0

add_library(freeRtosCoreConfiguration INTERFACE)
target_include_directories(freeRtosCoreConfiguration INTERFACE include)
target_link_libraries(freeRtosCoreConfiguration INTERFACE bsp bspMcu)
