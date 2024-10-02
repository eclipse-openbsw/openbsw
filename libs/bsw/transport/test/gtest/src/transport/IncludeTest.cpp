// Copyright 2024 Accenture.

/**
 * Contains
 * \file
 * \ingroup
 */

#ifndef INCLUDETEST_CPP_
#define INCLUDETEST_CPP_

#include "transport/AbstractTransportLayer.h"
#include "transport/DestituteTransportMessageProvider.h"
#include "transport/FunctionalTransportMessageSendJob.h"
#include "transport/IDataProgressListener.h"
#include "transport/ITransportMessageListener.h"
#include "transport/ITransportMessageProcessedListener.h"
#include "transport/ITransportMessageProvider.h"
#include "transport/ITransportMessageProvidingListener.h"
#include "transport/QueuedTransportLayer.h"
#include "transport/SimpleTransportMessageProvider.h"
#include "transport/TransportJob.h"
#include "transport/TransportMessage.h"
#include "transport/TransportMessageSendJob.h"

#include <gtest/gtest.h>

namespace
{
TEST(IncludeTest, CheckIncludes)
{
    // This test currently only checks if the includes are working.
}

} // anonymous namespace

#endif /* INCLUDETEST_CPP_ */
