// Copyright 2024 Accenture.

#pragma once

#include "can/transceiver/ICanTransceiver.h"

#include <platform/estdint.h>

namespace can
{
/**
 * For gathering CAN Rx/Tx Stats.
 */
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
