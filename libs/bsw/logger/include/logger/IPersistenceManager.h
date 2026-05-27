/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <etl/span.h>
#include <etl/uncopyable.h>

#include <platform/estdint.h>

namespace logger
{
class IPersistenceManager : public ::etl::uncopyable
{
public:
    virtual bool writeMapping(::etl::span<uint8_t const> const& src) const          = 0;
    virtual ::etl::span<uint8_t const> readMapping(::etl::span<uint8_t> dest) const = 0;

protected:
    IPersistenceManager() {}
};

} // namespace logger
