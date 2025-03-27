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
    MOCK_METHOD1(mergeWithBitField, void(BitFieldFilter&));
    MOCK_METHOD1(mergeWithStaticBitField, void(AbstractStaticBitFieldFilter&));
    MOCK_METHOD1(mergeWithInterval, void(IntervalFilter&));
};

} // namespace can
