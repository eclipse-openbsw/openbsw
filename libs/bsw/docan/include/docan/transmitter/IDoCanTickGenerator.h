// Copyright 2024 Accenture.

#pragma once

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
