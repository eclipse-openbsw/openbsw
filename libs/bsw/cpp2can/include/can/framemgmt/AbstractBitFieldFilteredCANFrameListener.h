/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

/**
 * Contains AbstractBitFieldFilteredCANFrameListener declaration.
 * \file AbstractBitFieldFilteredCANFrameListener.h
 * \ingroup framemgmt
 */
#pragma once

#include "can/filter/BitFieldFilter.h"
#include "can/framemgmt/ICANFrameListener.h"

namespace can
{
/**
 * ICANFrameListener that operates on a BitFieldFilter.
 *
 *
 * \see ICANFrameListener
 */
class AbstractBitFieldFilteredCANFrameListener : public ICANFrameListener
{
public:
    AbstractBitFieldFilteredCANFrameListener();

    /**
     * \see ICANFrameListener::getFilter()
     */
    IFilter& getFilter() override;

private:
    BitFieldFilter fFilter;
};

/*
 * inline implementation
 */
inline AbstractBitFieldFilteredCANFrameListener::AbstractBitFieldFilteredCANFrameListener()
: ICANFrameListener(), fFilter()
{}

// virtual
inline IFilter& AbstractBitFieldFilteredCANFrameListener::getFilter() { return fFilter; }

} // namespace can
