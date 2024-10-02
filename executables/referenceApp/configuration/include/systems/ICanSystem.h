// Copyright 2024 Accenture.

#ifndef GUARD_4BEC0C6E_710F_4954_B960_19239186B7F5
#define GUARD_4BEC0C6E_710F_4954_B960_19239186B7F5

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

#endif // GUARD_4BEC0C6E_710F_4954_B960_19239186B7F5
