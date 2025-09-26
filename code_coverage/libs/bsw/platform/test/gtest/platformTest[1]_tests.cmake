add_test([=[estdint.sizes]=]  /home/jenkins/code_coverage/libs/bsw/platform/test/gtest/Debug/platformTest [==[--gtest_filter=estdint.sizes]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[estdint.sizes]=]  PROPERTIES WORKING_DIRECTORY /home/jenkins/code_coverage/libs/bsw/platform/test/gtest SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==] LABELS platformTest)
set(  platformTest_TESTS estdint.sizes)
