// Copyright 2024 Accenture.

/**
 * @class       CanPhyGeneric
 *
 * CanPhyGeneric formulates the generic interface that describes
 * access for a hardware-specific CAN-Transceiver
 */

#pragma once

#include <platform/estdint.h>

namespace bios
{
class CanPhy
{
public:
    enum ErrorCode
    {
        CAN_PHY_ERROR_NONE = 0,
        CAN_PHY_ERROR,
        CAN_PHY_ERROR_UNSUPPORTED
    };

    enum Mode
    {
        CAN_PHY_MODE_UNDEFINED = 0,
        CAN_PHY_MODE_STANDBY   = 1,
        CAN_PHY_MODE_ACTIVE    = 2
    };

    virtual void init(uint32_t id)               = 0;
    virtual bool setMode(Mode mode, uint32_t id) = 0;
    virtual Mode getMode(uint32_t id);
    virtual ErrorCode getPhyErrorStatus(uint32_t id) = 0;

protected:
    Mode fMode = CAN_PHY_MODE_UNDEFINED;

    CanPhy& operator=(CanPhy const&) = default;
};

inline CanPhy::Mode CanPhy::getMode(uint32_t /*id*/) { return fMode; }

class CanPhyDummy : public CanPhy
{
public:
    static CanPhyDummy& getInstance()
    {
        static CanPhyDummy phyDummy;
        return phyDummy;
    }

    void init(uint32_t id) override;
    bool setMode(Mode mode, uint32_t id) override;
    ErrorCode getPhyErrorStatus(uint32_t id) override;
};

inline void CanPhyDummy::init(uint32_t /*id*/) {}

inline bool CanPhyDummy::setMode(Mode const mode, uint32_t /*id*/)
{
    fMode = mode;
    return true;
}

inline CanPhyDummy::ErrorCode CanPhyDummy::getPhyErrorStatus(uint32_t /*id*/)
{
    return CAN_PHY_ERROR_UNSUPPORTED;
}

} /* namespace bios */
