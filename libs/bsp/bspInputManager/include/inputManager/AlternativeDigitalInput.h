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
 * Alternative Input.
 *
 */
#pragma once
#include "platform/estdint.h"

namespace bios
{
class AlternativeDigitalInput
{
public:
    AlternativeDigitalInput();
    void init(uint16_t port, uint16_t config, uint8_t init_state);
    uint8_t process(uint16_t port_in, uint16_t max_debounce, bool inv);

    bool getState() const { return static_cast<bool>(state); }

    bool getErEdge() const { return static_cast<bool>(edge_pos); }

    bool getFlEdge() const { return static_cast<bool>(edge_neg); }

private:
    uint32_t state      : 1;
    uint32_t edge_neg   : 1;
    uint32_t edge_pos   : 1;
    uint32_t status_old : 1;
    uint32_t reser      : 12;
    uint16_t debounced;
};

} // namespace bios
