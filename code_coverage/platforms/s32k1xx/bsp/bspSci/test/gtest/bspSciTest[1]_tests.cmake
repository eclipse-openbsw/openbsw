add_test([=[BspTest.IncludeCheck]=]  /home/jenkins/code_coverage/platforms/s32k1xx/bsp/bspSci/test/gtest/Debug/bspSciTest [==[--gtest_filter=BspTest.IncludeCheck]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[BspTest.IncludeCheck]=]  PROPERTIES WORKING_DIRECTORY /home/jenkins/code_coverage/platforms/s32k1xx/bsp/bspSci/test/gtest SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==] LABELS bspSciTest)
set(  bspSciTest_TESTS BspTest.IncludeCheck)
