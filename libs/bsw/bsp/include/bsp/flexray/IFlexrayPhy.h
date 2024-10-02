// Copyright 2024 Accenture.

/**
 * \ingroup bsp_flexray
 */
#ifndef GUARD_D2EAE788_E4B6_4BC8_962C_E68A681CE57A
#define GUARD_D2EAE788_E4B6_4BC8_962C_E68A681CE57A

#include "bsp/Bsp.h"

#include <util/types/Enum.h>

#include <estd/bitset.h>

namespace bsp
{
namespace flexray
{
/**
 * Interface for a Flexray phy.
 */
class IFlexrayPhy
{
    struct OperatingModes
    {
        enum type
        {
            MODE_RESET,
            MODE_STANDBY,
            MODE_SLEEP,
            MODE_NORMAL,
            MODE_POWER_OFF
        };
    };

    struct BranchModes
    {
        enum type
        {
            MODE_NORMAL,
            MODE_TX_ONLY,
            MODE_RX_ONLY,
            MODE_DISABLED,
            MODE_TX_ON_NOISE // only TX enabled on a noisy branch, RX will be enabled
                             // automatically after noise is reduced
        };
    };

public:
    using OperatingMode = ::util::types::Enum<OperatingModes>;
    using BranchMode    = ::util::types::Enum<BranchModes>;
    using BranchMask    = ::estd::bitset<32>;

    /**
     * Synchronously resets the Flexray phy.
     *
     * In most cases this will require setting the RST pin to LOW for some microseconds and then
     * releasing it.
     */
    virtual void reset() = 0;

    virtual ::bsp::BspReturnCode setOperatingMode(OperatingMode mode) = 0;

    virtual ::bsp::BspReturnCode setBranchMode(BranchMask branches, BranchMode mode) = 0;

    virtual uint8_t getNumberOfBranches() const = 0;

protected:
    IFlexrayPhy& operator=(IFlexrayPhy const&) = default;
};

} // namespace flexray
} // namespace bsp

#endif /* GUARD_D2EAE788_E4B6_4BC8_962C_E68A681CE57A */
