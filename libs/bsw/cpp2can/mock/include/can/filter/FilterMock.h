// Copyright 2024 Accenture.

/**
 * Contains
 * \file
 * \ingroup
 */

#pragma once

#include "can/filter/IFilter.h"

#include <gmock/gmock.h>

namespace can
{
struct FilterMock : public IFilter
{
    MOCK_METHOD(void, add, (uint32_t), (override));
    MOCK_METHOD(void, add, (uint32_t, uint32_t), (override));
    MOCK_METHOD(bool, match, (uint32_t), (const, override));
    MOCK_METHOD(void, clear, (), (override));
    MOCK_METHOD(void, open, (), (override));
    MOCK_METHOD(void, acceptMerger, (IMerger&), (override));
};

} // namespace can
