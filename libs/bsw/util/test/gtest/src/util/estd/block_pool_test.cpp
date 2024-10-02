// Copyright 2024 Accenture.

#include <util/estd/block_pool.h>

#include <estd/array.h>

#include <gmock/gmock.h>

#include <algorithm>

using namespace ::testing;

TEST(BlockPool, Constructor)
{
    ::util::estd::declare::block_pool<5, 24> cut;
    EXPECT_TRUE(cut.full());
    EXPECT_FALSE(cut.empty());
    EXPECT_EQ(5U, cut.size());
    EXPECT_EQ(5U, cut.max_size());
    EXPECT_EQ(24U, cut.block_size());
}

TEST(BlockPool, ConstructorIncreasesMemorySizeIfWithBlocksLessThanPointerSize)
{
    ::util::estd::declare::block_pool<5, sizeof(void*)> cut;
    EXPECT_TRUE(cut.full());
    EXPECT_FALSE(cut.empty());
    EXPECT_EQ(5U, cut.size());
    EXPECT_EQ(5U, cut.max_size());
    EXPECT_EQ(sizeof(void*), cut.block_size());
}

TEST(BlockPool, AcquireAndReleaseBlocks)
{
    ::estd::array<uint8_t, 50> blockMemory;
    uint8_t* blockStart                = &blockMemory[0];
    uint8_t variableOutsideBlockMemory = 0;
    ::util::estd::block_pool cut(blockStart, 50, 8);
    for (uint8_t loop = 0; loop < 2U; ++loop)
    {
        {
            ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
            EXPECT_THROW(cut.release(&variableOutsideBlockMemory), ::estd::assert_exception);
        }
        EXPECT_FALSE(cut.contains(&variableOutsideBlockMemory));
        EXPECT_TRUE(cut.contains(blockStart));
        EXPECT_EQ(0U, cut.index_of(blockStart));
        EXPECT_EQ(0U, cut.index_of(blockStart + 7));
        EXPECT_EQ(1U, cut.index_of(blockStart + 8));
        EXPECT_EQ(1U, cut.index_of(blockStart + 15));
        EXPECT_TRUE(cut.contains(blockStart + 47));
        EXPECT_EQ(5U, cut.index_of(blockStart + 40));
        EXPECT_EQ(5U, cut.index_of(blockStart + 47));
        EXPECT_FALSE(cut.contains(blockStart + 48));
        EXPECT_FALSE(cut.contains(blockStart + 49));
        EXPECT_FALSE(cut.contains(blockStart + 50));
        uint8_t* blocks[6];
        uint8_t idx = 0;
        while (idx < 5)
        {
            blocks[idx] = cut.acquire();
            EXPECT_THAT(blocks[idx], NotNull());
            EXPECT_FALSE(cut.full());
            EXPECT_FALSE(cut.empty());
            ++idx;
        }
        blocks[idx] = cut.acquire();
        EXPECT_THAT(blocks[idx], NotNull());
        EXPECT_FALSE(cut.full());
        EXPECT_TRUE(cut.empty());
        // check whether the blocks are in order
        ::std::stable_sort(blocks, blocks + 6U);
        for (idx = 0; idx < 6; ++idx)
        {
            EXPECT_EQ(blockStart + 8U * idx, blocks[idx]);
        }
        {
            ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
            EXPECT_THROW(cut.acquire(), ::estd::assert_exception);
        }
        cut.clear();
    }
}
