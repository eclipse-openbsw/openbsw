add_executable(bspInterruptsTest src/InterruptTest.cpp src/IncludeTest.cpp)

target_include_directories(bspInterruptsTest PRIVATE)

target_link_libraries(bspInterruptsTest PRIVATE bspInterrupts gmock_main)

gtest_discover_tests(bspInterruptsTest PROPERTIES LABELS "bspInterruptsTest")
