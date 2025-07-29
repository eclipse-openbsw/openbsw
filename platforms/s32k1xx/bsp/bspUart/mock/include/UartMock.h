#pragma once

#include "bsp/uart/UartApi.h"

#include <gmock/gmock.h>

namespace bsp
{

namespace bsp
{

class UartMock : public UartApi
{
public:
    MOCK_METHOD1(write, size_t(::etl::span<uint8_t const> const data));
    MOCK_METHOD1(read, size_t(::etl::span<uint8_t> data));
};

} // namespace bsp
} // namespace bsp
