// Copyright 2024 Accenture.

#ifndef GUARD_788E1E7D_4B7E_404B_AE2F_BF12EC9FAF80
#define GUARD_788E1E7D_4B7E_404B_AE2F_BF12EC9FAF80

#include "bsp/Bsp.h"

#include <util/types/Enum.h>

#include <estd/bitset.h>
#include <platform/estdint.h>

namespace bsp
{
namespace can
{
/**
 * Interface for CAN phy implementations.
 */
class ICanPhy
{
    struct OperatingModes
    {
        enum type
        {
            MODE_INVALID,
            MODE_STANDBY,
            MODE_SLEEP,
            MODE_LISTEN_ONLY,
            MODE_NORMAL
        };
    };

public:
    using ChannelMask = ::estd::bitset<32>;

    using OperatingMode = ::util::types::Enum<OperatingModes>;

    virtual ::bsp::BspReturnCode
    setOperatingMode(OperatingMode mode, ChannelMask channels = ChannelMask())
        = 0;

    virtual OperatingMode operatingMode(uint8_t channel) const = 0;

    virtual bool hasError(uint8_t channel) const = 0;

    virtual uint8_t numberOfChannels() const = 0;

protected:
    ICanPhy& operator=(ICanPhy const&) = default;
};

} // namespace can
} // namespace bsp

#endif /* GUARD_788E1E7D_4B7E_404B_AE2F_BF12EC9FAF80 */
