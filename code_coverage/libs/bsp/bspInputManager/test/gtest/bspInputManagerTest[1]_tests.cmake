add_test([=[BspTest.IncludeCheck]=]  /home/jenkins/code_coverage/libs/bsp/bspInputManager/test/gtest/Debug/bspInputManagerTest [==[--gtest_filter=BspTest.IncludeCheck]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[BspTest.IncludeCheck]=]  PROPERTIES WORKING_DIRECTORY /home/jenkins/code_coverage/libs/bsp/bspInputManager/test/gtest SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==] LABELS bspInputManagerTest)
set(  bspInputManagerTest_TESTS BspTest.IncludeCheck)
