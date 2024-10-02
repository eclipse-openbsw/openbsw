add_executable(
    estdTest
    src/estd/algorithm_test.cpp
    src/estd/array_test.cpp
    src/estd/assert_test.cpp
    src/estd/big_endian_test.cpp
    src/estd/bitset_test.cpp
    src/estd/chrono_test.cpp
    src/estd/constructor_test.cpp
    src/estd/deque_test.cpp
    src/estd/forward_list_test.cpp
    src/estd/function_test.cpp
    src/estd/include_test.cpp
    src/estd/indestructible_test.cpp
    src/estd/index_sequence_test.cpp
    src/estd/iterator_test.cpp
    src/estd/limited_test.cpp
    src/estd/little_endian_test.cpp
    src/estd/math_test.cpp
    src/estd/memory_test.cpp
    src/estd/multi_slice_test.cpp
    src/estd/object_pool_test.cpp
    src/estd/optional_test.cpp
    src/estd/ordered_map_test.cpp
    src/estd/ordered_vector_test.cpp
    src/estd/priority_queue_test.cpp
    src/estd/queue_test.cpp
    src/estd/result_test.cpp
    src/estd/ratio_test.cpp
    src/estd/ring_test.cpp
    src/estd/singleton_test.cpp
    src/estd/slice_test.cpp
    src/estd/stack_test.cpp
    src/estd/string_test.cpp
    src/estd/string_view_test.cpp
    src/estd/string_view_tokenize_test.cpp
    src/estd/tiny_ring_test.cpp
    src/estd/type_traits_test.cpp
    src/estd/type_utils_test.cpp
    src/estd/typed_mem_test.cpp
    src/estd/va_list_ref_test.cpp
    src/estd/variant_test.cpp
    src/estd/vec_tests.cpp
    src/estd/vector_test.cpp
    src/estd/tuple_test.cpp
    src/estd/type_list_test.cpp
    src/internal/TestClassCalls.cpp)

target_include_directories(estdTest PRIVATE include)

target_link_libraries(estdTest PRIVATE estd estdMock gmock_main platform)

target_compile_options(estdTest PRIVATE -Wno-array-bounds)

gtest_discover_tests(estdTest PROPERTIES LABELS "estdTest")
