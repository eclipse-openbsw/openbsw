#include "bsp/Uart.h"

#include <io/Io.h>
#include <mcu/mcu.h>

namespace bsp
{

enum class Uart::Id : size_t
{
    TERMINAL,
    INVALID,
};

static constexpr size_t NUMBER_OF_UARTS = static_cast<size_t>(Uart::Id::INVALID);

} // namespace bsp
