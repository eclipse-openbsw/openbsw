add_test([=[BspTest.IncludeCheck]=]  /home/jenkins/code_coverage/platforms/s32k1xx/bsp/bspIo/test/gtest/Debug/bspIoTest [==[--gtest_filter=BspTest.IncludeCheck]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[BspTest.IncludeCheck]=]  PROPERTIES WORKING_DIRECTORY /home/jenkins/code_coverage/platforms/s32k1xx/bsp/bspIo/test/gtest SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==] LABELS bspIoTest)
set(  bspIoTest_TESTS BspTest.IncludeCheck)
