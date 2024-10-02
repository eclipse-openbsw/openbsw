// Copyright 2024 Accenture.

/**
 * \ingroup async
 */
#ifndef GUARD_194C32A8_DA3A_4AD8_BB52_7ACC8D4CEBEC
#define GUARD_194C32A8_DA3A_4AD8_BB52_7ACC8D4CEBEC

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

#endif // GUARD_194C32A8_DA3A_4AD8_BB52_7ACC8D4CEBEC
