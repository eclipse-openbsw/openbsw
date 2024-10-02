// Copyright 2024 Accenture.

/**
 * Contains
 * \file
 * \ingroup
 */

#ifndef GUARD_F5E2F172_CF68_49B3_822F_97C00A368ED0
#define GUARD_F5E2F172_CF68_49B3_822F_97C00A368ED0

#include "can/framemgmt/ICANFrameListener.h"
#include "can/framemgmt/IFilteredCANFrameSentListener.h"

#include <gmock/gmock.h>

namespace can
{
struct FilteredCANFrameSentListenerMock : public IFilteredCANFrameSentListener
{
    MOCK_METHOD1(canFrameSent, void(CANFrame const&));
    MOCK_METHOD0(getFilter, IFilter&());
};

} // namespace can

#endif /* GUARD_F5E2F172_CF68_49B3_822F_97C00A368ED0 */
