// Copyright 2024 Accenture.

#ifndef GUARD_4790E190_76FC_488F_AD2D_50C90F49A61D
#define GUARD_4790E190_76FC_488F_AD2D_50C90F49A61D

#include "bsp/lin/ILinPhy.h"

namespace bsp
{
namespace lin
{
class LinPhy : public ILinPhy
{
public:
    LinPhy();

    /* ILinPhy */

    void init(uint32_t channel = 0U) override;

    void setMode(Mode mode, uint32_t channel = 0U) override = 0;

    Mode getMode(uint32_t channel = 0U) override;

    ErrorCode getPhyErrorStatus(uint32_t channel = 0U) override = 0;

protected:
    Mode fMode;

    LinPhy& operator=(LinPhy const&) = default;
};

//
// implementation
//

inline LinPhy::LinPhy() : fMode(LIN_PHY_MODE_UNDEFINED) {}

inline void LinPhy::init(uint32_t /*channel*/) {}

inline ILinPhy::Mode LinPhy::getMode(uint32_t /*channel*/) { return fMode; }

} // namespace lin
} // namespace bsp

#endif /* GUARD_4790E190_76FC_488F_AD2D_50C90F49A61D */
