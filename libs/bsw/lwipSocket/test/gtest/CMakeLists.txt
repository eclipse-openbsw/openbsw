add_executable(
    lwipSocketTest
    src/lwipSocket/LwipSocketTest.cpp)

target_link_libraries(lwipSocketTest PRIVATE lwipSocket gtest_main lwipcore)

target_include_directories(lwipSocket PRIVATE
    "${OPENBSW_APP_DIR}/lwipConfiguration/include"
    "${OPENBSW_PLATFORM_DIR}/lwipSysArch/include"
)

gtest_discover_tests(lwipSocketTest PROPERTIES LABELS "lwipSocketTest")
