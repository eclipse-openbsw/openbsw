add_library(bspEepromDriver src/eeprom/FlashEepromDriver.cpp)
target_include_directories(bspEepromDriver PUBLIC include)
target_link_libraries(
    bspEepromDriver
    PUBLIC bspMcu bsp
    PRIVATE platform)
