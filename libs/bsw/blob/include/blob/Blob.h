/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "blob/Config.h"
#include "blob/Header.h"
#include "blob/Logger.h"

#include <etl/iterator.h>
#include <etl/span.h>
#include <util/logger/Logger.h>

namespace blob
{
namespace logger = ::util::logger;

class Blob
{
public:
    static constexpr uint32_t VERSION   = 1U;
    static constexpr uint32_t MAGIC     = 0xDEADBEEFU;
    static constexpr size_t HEADER_SIZE = sizeof(Header);

    struct Iterator : ::etl::iterator<ETL_OR_STD::input_iterator_tag, Config>
    {
        explicit Iterator(::etl::span<uint8_t const> const b) : _b(b) {}

        value_type operator*() const { return Config::from_bytes(_b); }

        value_type operator->() const { return **this; }

        Iterator& operator++()
        {
            auto const config = **this;
            _b.advance(config.data.size());
            if ((_b.empty()) || (config.data.empty()))
            {
                _b = {};
            }
            return *this;
        }

        bool operator==(Iterator const& other) const
        {
            return (_b.size() == other._b.size()) && (_b.data() == other._b.data());
        }

        bool operator!=(Iterator const& other) const { return !(*this == other); }

    private:
        ::etl::span<uint8_t const> _b;
    };

    explicit Blob(::etl::span<uint8_t const> const b) : _blob()
    {
        ::blob::Header header;

        if (!::blob::checkHeader(b, header))
        {
            logger::Logger::error(logger::BLOB, "Failed to init blob due to invalid header");
            _blob = {};

            return;
        }

        auto const blobSize = header.size;
        if ((b.size() - HEADER_SIZE) != blobSize)
        {
            logger::Logger::error(
                logger::BLOB,
                "Failed to init blob due to invalid blob size (expected=%u,actual=%u)",
                static_cast<uint32_t>(blobSize),
                static_cast<uint32_t>(b.size() - HEADER_SIZE));

            _blob = {};

            return;
        }

        _blob = b.subspan(HEADER_SIZE);
    }

    Iterator begin() const { return Iterator(_blob); }

    Iterator end() const { return Iterator({}); }

private:
    ::etl::span<uint8_t const> _blob;
};

// [Load]
::etl::span<uint8_t const> load(::etl::span<uint8_t const> const mem);
// [Load]
} // namespace blob
