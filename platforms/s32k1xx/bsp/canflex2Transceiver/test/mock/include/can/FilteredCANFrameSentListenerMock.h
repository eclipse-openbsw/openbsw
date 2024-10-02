// Copyright 2024 Accenture.

#ifndef GUARD_AC2E0C66_F767_4982_BA09_8B8A543261F9
#define GUARD_AC2E0C66_F767_4982_BA09_8B8A543261F9

#include <can/framemgmt/IFilteredCANFrameSentListener.h>

#include <platform/estdint.h>

#include <gmock/gmock.h>

namespace can
{
class FilteredCANFrameSentListenerMock : public ::can::IFilteredCANFrameSentListener
{
public:
    MOCK_METHOD1(canFrameSent, void(CANFrame const&));
    MOCK_METHOD0(getFilter, IFilter&());
};

} // namespace can

#endif /* GUARD_AC2E0C66_F767_4982_BA09_8B8A543261F9 */
