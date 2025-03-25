// Copyright 2024 Accenture.

/**
 * Includes AbstractIntervalFilteredCANFrameListener.
 * \file AbstractIntervalFilteredCANFrameListener.h
 * \ingroup framemgmt
 */
#pragma once

#include "can/filter/IntervalFilter.h"
#include "can/framemgmt/ICANFrameListener.h"

namespace can
{
/**
 * ICANFrameListener that operates on a IntervalFilter.
 *
 *
 * \see ICANFrameListener
 */
class AbstractIntervalFilteredCANFrameListener : public ICANFrameListener
{
public:
    AbstractIntervalFilteredCANFrameListener();

    /**
     * \see ICANFrameListener::getFilter()
     */
    IFilter& getFilter() override;

protected:
    IntervalFilter fFilter;
};

/*
 * inline implementation
 */
inline AbstractIntervalFilteredCANFrameListener::AbstractIntervalFilteredCANFrameListener()
: ICANFrameListener(), fFilter()
{}

// virtual
inline IFilter& AbstractIntervalFilteredCANFrameListener::getFilter() { return fFilter; }

} // namespace can
