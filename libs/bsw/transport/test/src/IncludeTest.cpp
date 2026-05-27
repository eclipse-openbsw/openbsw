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
 * Contains
 * \file
 * \ingroup
 */

#ifndef INCLUDETEST_CPP_
#define INCLUDETEST_CPP_

// IWYU pragma: begin_keep
#include "transport/AbstractTransportLayer.h"
#include "transport/ITransportMessageListener.h"
#include "transport/ITransportMessageProcessedListener.h"
#include "transport/ITransportMessageProvider.h"
#include "transport/ITransportMessageProvidingListener.h"
#include "transport/TransportMessage.h"
#include "transport/TransportMessageSendJob.h"
// IWYU pragma: end_keep

#include <gtest/gtest.h>

namespace
{

TEST(IncludeTest, CheckIncludes)
{
    // This test currently only checks if the includes are working.
}

} // anonymous namespace

#endif /* INCLUDETEST_CPP_ */
