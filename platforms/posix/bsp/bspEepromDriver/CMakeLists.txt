add_library(bspEepromDriver src/eeprom/EepromDriver.cpp)

target_include_directories(bspEepromDriver PUBLIC include)

target_link_libraries(bspEepromDriver PUBLIC bspConfiguration bsp)
