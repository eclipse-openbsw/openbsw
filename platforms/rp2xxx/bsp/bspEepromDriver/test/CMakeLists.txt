add_executable(bspEepromDriverTestRp2xxx src/IncludeTest.cpp)

target_include_directories(bspEepromDriverTestRp2xxx PRIVATE ../include)

target_link_libraries(bspEepromDriverTestRp2xxx PRIVATE gtest_main bspIo
                                                        utbspMcu util)

gtest_discover_tests(bspEepromDriverTestRp2xxx
                     PROPERTIES LABELS "bspEepromDriverTestRp2xxx")
