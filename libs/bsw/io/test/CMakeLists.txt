add_executable(
    ioTest
    src/io/BufferedWriterTest.cpp
    src/io/ForwardingReaderTest.cpp
    src/io/JoinReaderTest.cpp
    src/io/MemoryQueueTest.cpp
    src/io/SplitWriterTest.cpp
    src/io/VariantQueueTest.cpp)

target_link_libraries(ioTest PRIVATE io ioMock)

gtest_discover_tests(ioTest PROPERTIES LABELS "ioTest")
