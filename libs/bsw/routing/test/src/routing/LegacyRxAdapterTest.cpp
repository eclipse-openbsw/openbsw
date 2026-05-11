/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "routing/LegacyRxAdapter.h"

#include "routing/ErrorHandler.h"
#include "routing/definition.h"

#include <etl/optional.h>
#include <io/MemoryQueue.h>

#include <gmock/gmock.h>

namespace
{
using namespace ::testing;

template<size_t CAPACITY = 1024, size_t MAX_ELEMENT_SIZE = 128>
struct LegacyRxAdapterTest
{
    using Queue     = ::io::MemoryQueue<CAPACITY, MAX_ELEMENT_SIZE>;
    using Reader    = ::io::MemoryQueueReader<Queue>;
    using Writer    = ::io::MemoryQueueWriter<Queue>;
    using RxAdapter = ::routing::LegacyRxAdapter<MAX_ELEMENT_SIZE>;

    LegacyRxAdapterTest(::etl::span<::routing::Definition> defs)
    : rxQueue(), rxReader(rxQueue), rxWriter(rxQueue)
    {
        routing::load(rxAdapterTable, defs, 0, rxAdapterTableMem);
        (void)rxAdapter.emplace(rxReader, rxAdapterTable, ::routing::ErrorHandler());
    }

    Queue rxQueue;
    Reader rxReader;
    Writer rxWriter;

    ::routing::RxAdapterTableMem rxAdapterTableMem;
    ::routing::RxAdapterTable rxAdapterTable;

    ::etl::optional<RxAdapter> rxAdapter;
};

/**
 * \desc
 * The max_size function returns the correct value.
 */
TEST(LegacyRxAdapterTest, max_size)
{
    constexpr size_t CAPACITY         = 128;
    constexpr size_t MAX_ELEMENT_SIZE = 16;

    LegacyRxAdapterTest<CAPACITY, MAX_ELEMENT_SIZE> rxAdapterTest({});
    auto& rxAdapter = *rxAdapterTest.rxAdapter;
    EXPECT_EQ(MAX_ELEMENT_SIZE, rxAdapter.maxSize());
}

} // namespace
