// Copyright 2025 Accenture.

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
