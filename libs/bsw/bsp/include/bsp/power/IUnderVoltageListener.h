// Copyright 2024 Accenture.

#ifndef GUARD_C1379F92_B8FD_46D7_A0DF_0C14DC89B4C3
#define GUARD_C1379F92_B8FD_46D7_A0DF_0C14DC89B4C3

#include <estd/forward_list.h>

namespace bsp
{
/**
 * Interface every class has to implement which needs to be notified if undervoltage occured.
 */
class IUnderVoltageListener : public ::estd::forward_list_node<IUnderVoltageListener>
{
public:
    /**
     * Notifies the listener that undervoltage occurred
     */
    virtual void undervoltageOccurred() = 0;
};

} /* namespace bsp */

#endif /* GUARD_C1379F92_B8FD_46D7_A0DF_0C14DC89B4C3 */
