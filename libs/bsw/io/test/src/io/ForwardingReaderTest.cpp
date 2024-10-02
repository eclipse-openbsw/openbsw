// Copyright 2024 Accenture.

#include "io/ForwardingReader.h"

#include "io/IReaderMock.h"
#include "io/IWriterMock.h"

#include <estd/optional.h>

#include <gmock/gmock.h>

#include <cstddef>
#include <cstdint>

using namespace ::testing;

namespace
{
struct ForwardingReaderTest : ::testing::Test
{
    ForwardingReaderTest() = default;

    StrictMock<::io::IReaderMock> _reader;
    StrictMock<::io::IWriterMock> _writer;
    ::io::ForwardingReader _forward{_reader, _writer};
};

/**
 * \refs:   SMD_io_ForwardingReader
 * \desc
 * This test makes sure that failedAllocations is zero after construction.
 */
TEST_F(ForwardingReaderTest, failed_allocations_is_zero_after_construction)
{
    EXPECT_EQ(0, _forward.failedAllocations);
}

/**
 * \refs:   SMD_io_ForwardingReader
 * \desc
 * This test makes sure that when calling maxSize() this call is forwarded to the source.
 */
TEST_F(ForwardingReaderTest, max_size_of_source_is_used)
{
    EXPECT_CALL(_reader, maxSize()).WillOnce(Return(100));
    EXPECT_EQ(100, _forward.maxSize());
}

/**
 * \refs:   SMD_io_ForwardingReader
 * \desc
 * This test assures that allocate() is not called if peek() on the source reader returns an empty
 * slice.
 */
TEST_F(ForwardingReaderTest, allocate_not_called_when_no_data_available)
{
    EXPECT_CALL(_writer, allocate(_)).Times(0);
    EXPECT_CALL(_reader, peek()).WillOnce(Return(::estd::slice<uint8_t>{}));
    auto data = _forward.peek();
    EXPECT_EQ(0, data.size());
}

/**
 * \refs:   SMD_io_ForwardingReader
 * \desc
 * This test assures that commit() is called even if peek() on the source reader returns an empty
 * slice.
 */
TEST_F(ForwardingReaderTest, commit_called_when_no_data_available)
{
    EXPECT_CALL(_writer, allocate(_)).Times(0);
    EXPECT_CALL(_reader, peek()).WillOnce(Return(::estd::slice<uint8_t>{}));
    auto data = _forward.peek();
    EXPECT_EQ(0, data.size());
    EXPECT_CALL(_writer, commit());
    EXPECT_CALL(_reader, release());
    _forward.release();
}

/**
 * \refs:   SMD_io_ForwardingReader
 * \desc
 * This test assures that allocate() is called with the size that the peek() on the source
 * reader returned.
 */
TEST_F(ForwardingReaderTest, allocate_called_with_size_of_source_data)
{
    uint8_t buffer[10];
    ::estd::slice<uint8_t> sourceData = decltype(sourceData)::from_pointer(buffer, sizeof(buffer));
    ASSERT_EQ(10, sourceData.size());

    uint8_t destinationBuffer[10]{0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    InSequence seq;
    EXPECT_CALL(_reader, peek()).WillOnce(Return(sourceData));
    EXPECT_CALL(_writer, allocate(sourceData.size()))
        .WillOnce(Return(::estd::slice<uint8_t>::from_pointer(
            &destinationBuffer[0], sizeof(destinationBuffer))));
    auto data = _forward.peek();
    EXPECT_EQ(sourceData.size(), data.size());
    // make sure failedAllocations is still zero
    EXPECT_EQ(0, _forward.failedAllocations);
}

/**
 * \refs:   SMD_io_ForwardingReader
 * \desc
 * This test assures failedAllocations is increased if source data is available but allocate()
 * returns an empty slice.
 */
TEST_F(ForwardingReaderTest, failed_allocations_is_increased_when_allocate_returns_empty_slice)
{
    uint8_t buffer[10];
    ::estd::slice<uint8_t> sourceData = decltype(sourceData)::from_pointer(buffer, sizeof(buffer));
    ASSERT_EQ(10, sourceData.size());

    InSequence seq;
    EXPECT_CALL(_reader, peek()).WillOnce(Return(sourceData));
    EXPECT_CALL(_writer, allocate(sourceData.size())).WillOnce(Return(::estd::slice<uint8_t>{}));

    auto data = _forward.peek();
    EXPECT_EQ(sourceData.size(), data.size());
    EXPECT_EQ(1, _forward.failedAllocations);
}

/**
 * \refs:   SMD_io_ForwardingReader
 * \desc
 * This test assures the input data is copied to the destination writer.
 */
TEST_F(ForwardingReaderTest, data_is_forwarded_on_release)
{
    uint8_t sourceBuffer[10]{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    ::estd::slice<uint8_t> sourceData
        = decltype(sourceData)::from_pointer(sourceBuffer, sizeof(sourceBuffer));
    ASSERT_EQ(10, sourceData.size());

    uint8_t destinationBuffer[10]{0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    InSequence seq;
    EXPECT_CALL(_reader, peek()).WillOnce(Return(sourceData));
    EXPECT_CALL(_writer, allocate(sourceData.size()))
        .WillOnce(Return(::estd::slice<uint8_t>::from_pointer(
            &destinationBuffer[0], sizeof(destinationBuffer))));
    auto data = _forward.peek();
    EXPECT_EQ(sourceData.size(), data.size());

    ASSERT_EQ(2, data[2]);

    EXPECT_CALL(_writer, commit());
    EXPECT_CALL(_reader, release());
    _forward.release();

    EXPECT_THAT(sourceData, ElementsAreArray(destinationBuffer));
    // Clear destination buffer
    memset(&destinationBuffer[0], 0x00, sizeof(destinationBuffer));

    // And make sure, data is not copied a second time when calling release again.
    EXPECT_CALL(_writer, commit());
    EXPECT_CALL(_reader, release());
    _forward.release();
    EXPECT_THAT(destinationBuffer, ElementsAre(0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
}

} // namespace
