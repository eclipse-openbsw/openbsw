add_executable(
    ioExamples
    buffered_writer.cpp
    forwarding_reader.cpp
    join_reader.cpp
    memory_queue.cpp
    split_writer.cpp
    variant_queue.cpp)

target_link_libraries(ioExamples PRIVATE io gmock_main)
