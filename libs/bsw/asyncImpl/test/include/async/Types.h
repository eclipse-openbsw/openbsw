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

#include "async/RunnableMock.h"

namespace async
{
using ContextType   = uint8_t;
using EventMaskType = uint32_t;
using RunnableType  = RunnableMock;

} // namespace async
