/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

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
