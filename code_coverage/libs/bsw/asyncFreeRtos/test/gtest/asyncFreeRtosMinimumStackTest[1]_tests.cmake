add_test([=[TaskInitializerStackTest.testStack]=]  /home/jenkins/code_coverage/libs/bsw/asyncFreeRtos/test/gtest/Debug/asyncFreeRtosMinimumStackTest [==[--gtest_filter=TaskInitializerStackTest.testStack]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[TaskInitializerStackTest.testStack]=]  PROPERTIES WORKING_DIRECTORY /home/jenkins/code_coverage/libs/bsw/asyncFreeRtos/test/gtest SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==] LABELS asyncFreeRtosMinimumStackTest)
set(  asyncFreeRtosMinimumStackTest_TESTS TaskInitializerStackTest.testStack)
