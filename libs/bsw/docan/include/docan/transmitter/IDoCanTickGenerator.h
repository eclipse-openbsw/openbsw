// Copyright 2024 Accenture.

#ifndef GUARD_2F01A26B_240D_4A6A_B6D4_F02AD5CF0545
#define GUARD_2F01A26B_240D_4A6A_B6D4_F02AD5CF0545

namespace docan
{
/**
 * Interface for DoCan tick generator.
 */
class IDoCanTickGenerator
{
public:
    /**
     * Called to indicate that cyclic ticks should be triggered
     */
    virtual void tickNeeded() = 0;

private:
    IDoCanTickGenerator& operator=(IDoCanTickGenerator const&) = delete;
};

} // namespace docan

#endif // GUARD_2F01A26B_240D_4A6A_B6D4_F02AD5CF0545
