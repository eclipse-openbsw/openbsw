/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

namespace safety
{

/**
 * SafeIO ensures integrity of safety related IOs.
 *
 * It locks registers at startup and cyclically monitors other registers.
 */
class SafeIo
{
public:
    SafeIo() = default;

    // [PUBLIC_API_START]

    /**
     * Initializes the SafeIo.
     *
     * This function must be called once before the first call to cyclic().
     * It locks the configuration of safety relevant IO registers.
     * In case of a detected error, it sends an event to the SafeSupervisor.
     */
    void init();

    /**
     * Cyclically checks the configuration of safety relevant IO registers.
     *
     * This function must be called cyclically.
     * In case of a detected error, it sends an event to the SafeSupervisor.
     */
    void cyclic();

    // [PUBLIC_API_END]

private:
    bool _lastCheckState;
};

} // namespace safety
