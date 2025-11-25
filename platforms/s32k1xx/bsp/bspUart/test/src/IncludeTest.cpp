#include "bsp/Uart.h"

#include <gtest/gtest.h>

namespace
{
TEST(BspTest, IncludeCheck)
{
    bsp::Uart uartMock;
    uartMock.write(::etl::span<uint8_t const>{});
    uartMock.read(::etl::span<uint8_t>{});
}

} // anonymous namespace
