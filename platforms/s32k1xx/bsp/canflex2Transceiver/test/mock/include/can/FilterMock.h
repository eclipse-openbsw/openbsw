// Copyright 2024 Accenture.

#pragma once

#include <can/filter/IFilter.h>

#include <gmock/gmock.h>

namespace can
{
class FilterMock : public IFilter
{
public:
    MOCK_METHOD1(add, void(uint32_t));
    MOCK_METHOD2(add, void(uint32_t, uint32_t));
    MOCK_CONST_METHOD1(match, bool(uint32_t));
    MOCK_METHOD0(clear, void());
    MOCK_METHOD0(open, void());
    MOCK_METHOD1(acceptMerger, void(IMerger&));
};

} // namespace can

