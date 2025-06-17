// Copyright 2024 Accenture.

/**
 * Contains
 * \file
 * \ingroup
 */

#pragma once

#include "can/filter/IMerger.h"

#include <gmock/gmock.h>

namespace can
{
struct MergerMock : public IMerger
{
    MOCK_METHOD(void, mergeWithBitField, (BitFieldFilter&), (override));
    MOCK_METHOD(void, mergeWithStaticBitField, (AbstractStaticBitFieldFilter&), (override));
    MOCK_METHOD(void, mergeWithInterval, (IntervalFilter&), (override));
};

} // namespace can
