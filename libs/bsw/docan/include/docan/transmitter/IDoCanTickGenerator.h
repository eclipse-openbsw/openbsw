/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

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
