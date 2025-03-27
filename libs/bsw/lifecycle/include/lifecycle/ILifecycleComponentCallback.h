// Copyright 2024 Accenture.

/**
 * \file
 * \ingroup lifecycle
 */
#pragma once

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
