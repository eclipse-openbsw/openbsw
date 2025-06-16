// Copyright 2025 Accenture.

#pragma once

#include <storage/IStorage.h>

#include <gmock/gmock.h>

namespace storage
{

class IStorageMock : public IStorage
{
public:
    MOCK_METHOD1(process, void(StorageJob&));
};

} // namespace storage
