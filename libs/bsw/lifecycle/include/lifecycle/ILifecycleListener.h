// Copyright 2024 Accenture.

/**
 * \file
 * \ingroup lifecycle
 */
#ifndef GUARD_47F63FF4_0080_470A_A831_C136DBC2CFBE
#define GUARD_47F63FF4_0080_470A_A831_C136DBC2CFBE

#include "lifecycle/ILifecycleComponent.h"

#include <etl/intrusive_forward_list.h>

namespace lifecycle
{
class ILifecycleListener : public ::etl::forward_link<0>
{
public:
    virtual void
    lifecycleLevelReached(uint8_t level, ILifecycleComponent::Transition::Type transition)
        = 0;
    virtual ~ILifecycleListener() = default;
};

} // namespace lifecycle

#endif // LIFECYCLE_ILIFECYCLECOMPONENT_H_
