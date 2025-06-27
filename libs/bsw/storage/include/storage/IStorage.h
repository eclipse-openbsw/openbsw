// Copyright 2025 Accenture.

#pragma once

namespace storage
{

class StorageJob;

class IStorage
{
public:
    virtual void process(StorageJob& job) = 0;

protected:
    ~IStorage() = default;
};

} // namespace storage
