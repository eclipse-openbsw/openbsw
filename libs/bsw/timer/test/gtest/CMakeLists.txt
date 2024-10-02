add_executable(timerTest timer/TimerTest.cpp)

target_link_libraries(timerTest PRIVATE timer gmock_main)

gtest_discover_tests(timerTest PROPERTIES LABELS "timerTest")
