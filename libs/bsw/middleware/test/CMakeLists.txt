add_executable(
    middlewareTest
    src/concurrency/ConcurrencyDefinitions.cpp
    src/core/middleware_cluster_configuration_unittest.cpp
    src/core/middleware_connection_unittest.cpp
    src/core/middleware_db_manipulator_unittest.cpp
    src/core/middleware_logger_api.cpp
    src/core/middleware_message_unittest.cpp
    src/core/middleware_proxy_base_unittest.cpp
    src/core/middleware_skeleton_base_unittest.cpp
    src/logger/mock/LoggerMock.cpp
    src/os/OsDefinitions.cpp
    src/queue/middleware_queue_unittest.cpp
    src/time/mock/SystemTimerProviderMock.cpp)

target_include_directories(middlewareTest PUBLIC src)

target_link_libraries(middlewareTest PRIVATE middleware gmock gtest_main)

gtest_discover_tests(middlewareTest PROPERTIES LABELS "middlewareTest")
