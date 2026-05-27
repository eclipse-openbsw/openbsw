/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <storage/IStorage.h>

#include <gmock/gmock.h>

namespace storage
{

class IStorageMock : public IStorage
{
public:
    MOCK_METHOD(void, process, (StorageJob&));
};

} // namespace storage
