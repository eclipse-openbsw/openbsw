// Copyright 2024 Accenture.

#ifndef GUARD_7D7035F2_88B1_4AE5_A82C_FF43F3DA7174
#define GUARD_7D7035F2_88B1_4AE5_A82C_FF43F3DA7174

namespace bsp
{
/**
 * @class   IHardwareInfoProvider
 *
 * Interface for accessing the Hardware Infos
 */
class IHardwareInfoProvider
{
public:
    virtual bool isFlexrayHighVersion() = 0;

    virtual bool isMostAvailable() = 0;

    virtual bool isComBoard() = 0;

    virtual bool isDebugCanAvailable() = 0;

protected:
    IHardwareInfoProvider& operator=(IHardwareInfoProvider const&) = default;
};

} /* namespace bsp */

#endif /* GUARD_7D7035F2_88B1_4AE5_A82C_FF43F3DA7174 */
