add_library(
    util
    src/util/logger/StderrLogger.cpp
    src/util/logger/ComponentInfo.cpp
    src/util/logger/Logger.cpp
    src/util/logger/LevelInfo.cpp
    src/util/memory/BuddyMemoryManager.cpp
    src/util/stream/StdoutStream.cpp
    src/util/stream/TaggedOutputHelper.cpp
    src/util/stream/StdinStream.cpp
    src/util/stream/ByteBufferOutputStream.cpp
    src/util/stream/NormalizeLfOutputStream.cpp
    src/util/stream/StringBufferOutputStream.cpp
    src/util/stream/TaggedSharedOutputStream.cpp
    src/util/stream/NullOutputStream.cpp
    src/util/stream/SharedOutputStream.cpp
    src/util/stream/TaggedOutputStream.cpp
    src/util/estd/assert.cpp
    src/util/crc/LookupTable_0x1D.cpp
    src/util/crc/LookupTable_0x07.cpp
    src/util/crc/LookupTable_0x1021.cpp
    src/util/crc/LookupTable_0xF4ACFB13.cpp
    src/util/crc/LookupTable_0x4C11DB7.cpp
    src/util/crc/LookupTable_0xCF.cpp
    src/util/crc/LookupTable_0x31.cpp
    src/util/crc/LookupTable_0x2F.cpp
    src/util/format/Vt100AttributedStringFormatter.cpp
    src/util/format/PrintfFormatter.cpp
    src/util/format/StringWriter.cpp
    src/util/format/PrintfFormatScanner.cpp
    src/util/format/AttributedString.cpp
    src/util/format/PrintfArgumentReader.cpp
    src/util/format/SharedStringWriter.cpp
    src/util/string/ConstString.cpp
    src/util/command/CommandContext.cpp
    src/util/command/GroupCommand.cpp
    src/util/command/SimpleCommand.cpp
    src/util/command/HelpCommand.cpp
    src/util/command/ParentCommand.cpp)

target_include_directories(util PUBLIC include)

target_link_libraries(util PUBLIC etl platform)

if (BUILD_UNIT_TESTS)
    target_compile_definitions(util PUBLIC ESTD_HAS_EXCEPTIONS
                                           ESTL_ASSERT_MESSAGE_ALL)

    add_library(utilMock mock/gmock/src/util/StdIoMock.cpp
                         mock/gmock/src/util/logger/TestConsoleLogger.cpp)

    target_include_directories(utilMock PUBLIC mock/gmock/include)

    target_link_libraries(utilMock PUBLIC util etl)
endif ()
