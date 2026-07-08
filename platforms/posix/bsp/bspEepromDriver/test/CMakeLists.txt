add_executable(bspEepromDriverTest src/eeprom/EepromDriverTest.cpp
                                   ../src/eeprom/EepromDriver.cpp)

target_include_directories(bspEepromDriverTest PRIVATE ../include)

target_link_libraries(bspEepromDriverTest PRIVATE bsp bspConfiguration
                                                  gtest_main)

# The posix EepromDriver is backed by a single shared file (EEPROM_FILEPATH).
# gtest_discover_tests registers every test case as its own process and ctest
# runs them in parallel (CTEST_PARALLEL_LEVEL). Without a lock the concurrent
# EepromDriver constructors race on that shared file and can re-initialize it
# (fill it with 0xFF), clobbering data another test just wrote. Serialize the
# tests against each other with a RESOURCE_LOCK on the shared file.
gtest_discover_tests(
    bspEepromDriverTest PROPERTIES LABELS "bspEepromDriverTest" RESOURCE_LOCK
                                   "openbsw_posix_eeprom_ut_file")
