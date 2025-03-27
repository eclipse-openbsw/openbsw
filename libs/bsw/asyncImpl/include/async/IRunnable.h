// Copyright 2024 Accenture.

/**
 * \ingroup async
 */
#pragma once

#include "async/QueueNode.h"

namespace async
{
class IRunnable : public QueueNode<IRunnable>
{
public:
    virtual void execute() = 0;

protected:
    IRunnable& operator=(IRunnable const&) = default;
};

} // namespace async
