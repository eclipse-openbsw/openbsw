// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

#pragma once

namespace safety
{

class SafetyManager
{
public:
    SafetyManager() = default;
    void init();
    void run();
    void shutdown();
    void cyclic();
};

} // namespace safety
