add_executable(bspEepromDriverTestS32K1 src/IncludeTest.cpp)

target_include_directories(bspEepromDriverTestS32K1 PRIVATE ../include)

target_link_libraries(bspEepromDriverTestS32K1 PRIVATE gtest_main utbspIo
                                                       utbspMcu util)

gtest_discover_tests(bspEepromDriverTestS32K1
                     PROPERTIES LABELS "bspEepromDriverTestS32K1")
