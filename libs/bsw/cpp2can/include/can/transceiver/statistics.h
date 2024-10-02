// Copyright 2024 Accenture.

#ifndef GUARD_8791664A_4256_4BA3_B5CF_2710EB2102FE
#define GUARD_8791664A_4256_4BA3_B5CF_2710EB2102FE

#include "can/transceiver/ICanTransceiver.h"

#include <platform/estdint.h>

namespace can
{
struct Statistics
{
    Statistics() : errors(0U), rx(0U), tx(0U), rx_dropped(0U), tx_dropped(0U) {}

    uint32_t errors;
    uint32_t rx;
    uint32_t tx;
    uint32_t rx_dropped;
    uint32_t tx_dropped;
};

// Target/Driver specific implementation necessary.
extern Statistics get_statistics(ICanTransceiver& transceiver);

} // namespace can

#endif // GUARD_8791664A_4256_4BA3_B5CF_2710EB2102FE
