add_executable(bspOutputPwmTest src/IncludeTest.cpp)

target_include_directories(bspOutputPwmTest PRIVATE ../../include)

target_link_libraries(
    bspOutputPwmTest
    PRIVATE etl
            bspDynamicClient
            utbspConfiguration
            utbspIo
            utbspMcu
            util
            gtest_main)

gtest_discover_tests(bspOutputPwmTest PROPERTIES LABELS "bspOutputPwmTest")
