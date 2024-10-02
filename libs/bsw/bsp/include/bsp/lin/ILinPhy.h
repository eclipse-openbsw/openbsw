// Copyright 2024 Accenture.

#ifndef GUARD_57D8B37A_D490_49B2_B691_934E04A308E9
#define GUARD_57D8B37A_D490_49B2_B691_934E04A308E9

#include <platform/estdint.h>

namespace bsp
{
namespace lin
{
class ILinPhy
{
public:
    enum ErrorCode
    {
        LIN_PHY_ERROR,
        LIN_PHY_NO_ERROR,
        LIN_PHY_INVALID_STATE
    };

    enum Mode
    {
        LIN_PHY_MODE_UNDEFINED = 0,
        LIN_PHY_MODE_STANDBY   = 1,
        LIN_PHY_MODE_ACTIVE    = 2
    };

    virtual void init(uint32_t channel = 0U) = 0;

    virtual void setMode(Mode mode, uint32_t channel = 0U) = 0;

    virtual Mode getMode(uint32_t channel = 0U) = 0;

    virtual ErrorCode getPhyErrorStatus(uint32_t channel = 0U) = 0;

protected:
    ILinPhy& operator=(ILinPhy const&) = default;
};

} // namespace lin
} // namespace bsp

#endif /* GUARD_57D8B37A_D490_49B2_B691_934E04A308E9 */
