// Copyright 2024 Accenture.

/**
 * \file
 * \ingroup lifecycle
 */
#ifndef GUARD_059084CD_45D5_4CFD_930C_8A5BCBDAA184
#define GUARD_059084CD_45D5_4CFD_930C_8A5BCBDAA184

#include <async/Async.h>

namespace lifecycle
{
class ILifecycleComponent;

class ILifecycleComponentCallback
{
public:
    virtual void transitionDone(ILifecycleComponent& component) = 0;

    virtual ~ILifecycleComponentCallback() = default;

private:
    ILifecycleComponentCallback& operator=(ILifecycleComponentCallback const&) = delete;
};

} // namespace lifecycle

#endif // GUARD_059084CD_45D5_4CFD_930C_8A5BCBDAA184
