add_test([=[bspCharInputOutput.IncludeTest]=]  /home/jenkins/code_coverage/libs/bsp/bspCharInputOutput/test/gtest/Debug/bspCharInputOutputTest [==[--gtest_filter=bspCharInputOutput.IncludeTest]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[bspCharInputOutput.IncludeTest]=]  PROPERTIES WORKING_DIRECTORY /home/jenkins/code_coverage/libs/bsp/bspCharInputOutput/test/gtest SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==] LABELS bspCharInputOutputTest)
set(  bspCharInputOutputTest_TESTS bspCharInputOutput.IncludeTest)
