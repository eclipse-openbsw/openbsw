add_library(bspEepromDriver src/eeprom/EepromDriver.cpp)

target_include_directories(bspEepromDriver PUBLIC include)

target_link_libraries(bspEepromDriver PRIVATE bsp bspCore bspInterrupts bspMcu util etl)
