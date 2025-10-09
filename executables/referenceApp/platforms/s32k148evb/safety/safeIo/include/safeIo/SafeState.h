// Copyright 2025 Accenture.

#pragma once

namespace safety
{

/**
 * SafeState manages the safe state of safety relevant IOs.
 *
 * In case of a detected error, the SafeState can be used to put safety relevant IOs into a safe
 * state, e.g. by disabling outputs.
 */
class SafeState
{
public:
    // [PUBLIC_API_START]

    /**
     * Sets IOs into a safe state, e.g by disabling outputs.
     */
    static void enterSafeState();

    /**
     * Re-enables IOs.
     */
    static void leaveSafeState();

    // [PUBLIC_API_END]
};

} // namespace safety
