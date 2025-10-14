add_executable(
    utilTest
    src/util/defer/DeferTest.cpp
    src/util/estd/assert_test.cpp
    src/util/types/EnumTest.cpp
    src/util/logger/LevelInfoTest.cpp
    src/util/logger/ComponentInfoTest.cpp
    src/util/logger/LoggerTest.cpp
    src/util/string/ConstStringTest.cpp
    src/util/meta/BitmaskTest.cpp
    src/util/meta/BinaryValueTest.cpp
    src/util/IncludeTest.cpp
    src/util/command/GroupCommandTest.cpp
    src/util/command/ParentCommandTest.cpp
    src/util/command/HelpCommandTest.cpp
    src/util/command/SimpleCommandTest.cpp
    src/util/command/ICommandTest.cpp
    src/util/command/CommandContextTest.cpp
    src/util/crc/Crc8Test.cpp
    src/util/crc/LookupTableTest.cpp
    src/util/crc/CrcTest.cpp
    src/util/crc/XorTest.cpp
    src/util/crc/Crc32Test.cpp
    src/util/crc/CrcRegisterTest.cpp
    src/util/crc/ReflectTest.cpp
    src/util/crc/Crc16Test.cpp
    src/util/math/MovingAverageTest.cpp
    src/util/stream/TaggedSharedOutputStreamTest.cpp
    src/util/stream/SharedOutputStreamTest.cpp
    src/util/stream/TaggedOutputHelperTest.cpp
    src/util/stream/StdoutStreamTest.cpp
    src/util/stream/ByteBufferOutputStreamTest.cpp
    src/util/stream/StdinStreamTest.cpp
    src/util/stream/NormalizeLfOutputStreamTest.cpp
    src/util/stream/StringBufferOutputStreamTest.cpp
    src/util/stream/SharedOutputStreamResourceTest.cpp
    src/util/stream/NullOutputStreamTest.cpp
    src/util/stream/TaggedOutputStreamTest.cpp
    src/util/format/Vt100AttributedStringFormatterTest.cpp
    src/util/format/PrintfFormatScannerTest.cpp
    src/util/format/SharedStringWriterTest.cpp
    src/util/format/PrintfArgumentReaderTest.cpp
    src/util/format/PrintfFormatterTest.cpp
    src/util/format/StringWriterTest.cpp
    src/util/format/AttributedStringTest.cpp
    src/util/memory/BitTest.cpp
    src/util/memory/BuddyMemoryManagerTest.cpp)

target_include_directories(utilTest PRIVATE include)

target_link_libraries(utilTest PRIVATE util utilMock gmock_main etl)

target_compile_options(utilTest PRIVATE -Wno-array-bounds)

gtest_discover_tests(utilTest PROPERTIES LABELS "utilTest")
