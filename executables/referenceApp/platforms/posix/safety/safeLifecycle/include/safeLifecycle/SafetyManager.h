// Copyright 2025 Accenture.

#pragma once

namespace safety
{

class SafetyManager
{
public:
    SafetyManager();
    void init();
    void run();
    void shutdown();
    void cyclic();
};

} // namespace safety
