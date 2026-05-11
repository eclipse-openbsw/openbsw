# python -m blob binary -i routing.jsonl -c blob.routing.table | python -m blob
# header data -n CONFIGURATION_BLOB > include/blob/configuration.h

# python -m blob header config-type > include/blob/ConfigType.h
target_include_directories(blobConfiguration INTERFACE include)

add_executable(blobTest src/BlobTest.cpp)

target_link_libraries(blobTest PRIVATE blob blobConfiguration gmock_main)

gtest_discover_tests(blobTest PROPERTIES LABELS "blobTest")
