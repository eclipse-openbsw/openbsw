# Copyright 2024 Contributors to the Eclipse Foundation
#
# SPDX-License-Identifier: EPL-2.0

add_library(bspEepromDriver src/eeprom/FlashEepromDriver.cpp)
target_include_directories(bspEepromDriver PUBLIC include)
target_link_libraries(bspEepromDriver PUBLIC bspMcu bsp PRIVATE platform)
