#include "bsp/Uart.h"

namespace bsp
{

size_t Uart::write(::etl::span<uint8_t const> const data) { return data.size(); }

size_t Uart::read(::etl::span<uint8_t> data) { return data.size(); }

void Uart::init() {}

bool Uart::isInitialized() const { return true; }

bool Uart::waitForTxReady() { return true; }

} // namespace bsp
