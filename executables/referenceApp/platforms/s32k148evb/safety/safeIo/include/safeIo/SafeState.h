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
