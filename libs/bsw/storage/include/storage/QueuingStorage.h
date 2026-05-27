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

#include <async/util/Call.h>
#include <etl/intrusive_list.h>
#include <storage/IStorage.h>
#include <storage/StorageJob.h>

namespace storage
{

class QueuingStorage
: public IStorage
, private ::async::RunnableType
{
public:
    explicit QueuingStorage(IStorage& storage, ::async::ContextType context);
    ~QueuingStorage()                                = default;
    QueuingStorage(QueuingStorage const&)            = delete;
    QueuingStorage& operator=(QueuingStorage const&) = delete;

    void process(StorageJob& job) final;

private:
    void execute() final;

    IStorage& _storage;
    ::async::ContextType const _context;
    ::etl::intrusive_list<StorageJob, ::etl::bidirectional_link<0>> _jobs;
};

} // namespace storage
