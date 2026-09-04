/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/NetworkResourceMock.h"

#include <gtest/gtest.h>

namespace
{
using namespace ::testing;
using namespace ::ip;
using namespace ::someip;

/**
 * Make sure NetworkResource is not closed until _refCount == 0.
 */
TEST(NetworkResource, only_close_resources_without_refs)
{
    InSequence inSequence;

    NetworkResourceMock resourceMock;
    NetworkResource& resource = static_cast<NetworkResource&>(resourceMock);

    EXPECT_CALL(resourceMock, close());
    resource.tryClose();
    Mock::VerifyAndClearExpectations(&resourceMock);

    resource.incRefCounter();
    resource.tryClose();
    resource.incRefCounter();
    resource.tryClose();
    resource.decRefCounter();
    resource.decRefCounter();
    EXPECT_CALL(resourceMock, close());
    resource.tryClose();
}

} // anonymous namespace
