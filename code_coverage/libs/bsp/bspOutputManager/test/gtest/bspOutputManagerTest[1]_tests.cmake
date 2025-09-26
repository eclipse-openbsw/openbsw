add_test([=[BspTest.IncludeCheck]=]  /home/jenkins/code_coverage/libs/bsp/bspOutputManager/test/gtest/Debug/bspOutputManagerTest [==[--gtest_filter=BspTest.IncludeCheck]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[BspTest.IncludeCheck]=]  PROPERTIES WORKING_DIRECTORY /home/jenkins/code_coverage/libs/bsp/bspOutputManager/test/gtest SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==] LABELS bspOutputManagerTest)
set(  bspOutputManagerTest_TESTS BspTest.IncludeCheck)
