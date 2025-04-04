// Copyright 2024 Accenture.

#pragma once

#include <busid/BusId.h>

namespace can
{
class ICanTransceiver;

/**
 * Interface for lifecycle systems that provide access to CAN transceivers.
 */
class ICanSystem
{
protected:
    ~ICanSystem() = default;

public:
    /**
     * Returns a pointer to a ::can::ICanTransceiver for a given \p busId or nullptr if
     * there is no transceiver for the provided \p busId.
     */
    virtual ICanTransceiver* getCanTransceiver(uint8_t busId) = 0;
};
} // namespace can
