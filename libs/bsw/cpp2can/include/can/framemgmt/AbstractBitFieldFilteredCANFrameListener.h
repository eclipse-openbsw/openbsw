// Copyright 2024 Accenture.

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
