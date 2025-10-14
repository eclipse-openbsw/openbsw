add_executable(bspEepromDriverTest src/eeprom/EepromDriverTest.cpp
                                   ../src/eeprom/EepromDriver.cpp)

target_include_directories(bspEepromDriverTest PRIVATE ../include)

target_link_libraries(bspEepromDriverTest PRIVATE bsp utbspConfiguration
                                                  gtest_main)

gtest_discover_tests(bspEepromDriverTest PROPERTIES LABELS
                                                    "bspEepromDriverTest")
