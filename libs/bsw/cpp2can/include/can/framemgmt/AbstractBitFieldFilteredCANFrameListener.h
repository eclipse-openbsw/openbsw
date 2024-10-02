// Copyright 2024 Accenture.

/**
 * Contains AbstractBitFieldFilteredCANFrameListener declaration.
 * \file AbstractBitFieldFilteredCANFrameListener.h
 * \ingroup framemgmt
 */
#ifndef GUARD_994AA90C_A527_4A5B_9772_B1C8B19CCF5F
#define GUARD_994AA90C_A527_4A5B_9772_B1C8B19CCF5F

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

#endif /*GUARD_994AA90C_A527_4A5B_9772_B1C8B19CCF5F*/
