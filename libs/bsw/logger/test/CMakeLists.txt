add_executable(
    loggerTest
    src/logger/BufferedLoggerOutputClientTest.cpp
    src/logger/BufferedLoggerOutputTest.cpp
    src/logger/ComponentConfigTest.cpp
    src/logger/ComponentMappingTest.cpp
    src/logger/ConsoleEntryOutputTest.cpp
    src/logger/DefaultEntryFormatterTest.cpp
    src/logger/DefaultLoggerCommandTest.cpp
    src/logger/DefaultLoggerTimeTest.cpp
    src/logger/EntryBufferTest.cpp
    src/logger/EntrySerializerTest.cpp
    src/logger/LoggerBackgroundJobTest.cpp
    src/logger/PersistentComponentConfigTest.cpp
    src/logger/SharedStreamEntryOutputTest.cpp)

target_include_directories(loggerTest PRIVATE)

target_link_libraries(loggerTest PRIVATE logger loggerMock util)

gtest_discover_tests(loggerTest PROPERTIES LABELS "loggerTest")
