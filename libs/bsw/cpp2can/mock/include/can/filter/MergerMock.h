/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

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
