#pragma once

#include "bsp/uart/IUartApi.h"

#include <gmock/gmock.h>

namespace bsp
{

class UartMock : public IUartApi
{
public:
    MOCK_METHOD1(write, size_t(::etl::span<uint8_t const> const data));
    MOCK_METHOD1(read, size_t(::etl::span<uint8_t> data));
};

} // namespace bsp
