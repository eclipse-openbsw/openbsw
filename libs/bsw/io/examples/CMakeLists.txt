add_executable(
    ioExamples
    BufferedWriterExample.cpp
    ForwardingReaderExample.cpp
    JoinReaderExample.cpp
    MemoryQueueExample.cpp
    SplitWriterExample.cpp
    VariantQueueExample.cpp)

target_link_libraries(ioExamples PRIVATE io gmock_main)
