add_test([=[BspTest.IncludeCheck]=]  /home/jenkins/code_coverage/platforms/s32k1xx/bsp/bspCore/test/gtest/Debug/bspCoreTest [==[--gtest_filter=BspTest.IncludeCheck]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[BspTest.IncludeCheck]=]  PROPERTIES WORKING_DIRECTORY /home/jenkins/code_coverage/platforms/s32k1xx/bsp/bspCore/test/gtest SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==] LABELS bspCoreTest)
set(  bspCoreTest_TESTS BspTest.IncludeCheck)
