// Copyright 2024 Accenture.

/**
 * Contains
 * \file
 * \ingroup
 */

#ifndef GUARD_01FD90B4_C513_41B2_9E47_5E05243F8D0A
#define GUARD_01FD90B4_C513_41B2_9E47_5E05243F8D0A

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

#endif /* GUARD_01FD90B4_C513_41B2_9E47_5E05243F8D0A */
