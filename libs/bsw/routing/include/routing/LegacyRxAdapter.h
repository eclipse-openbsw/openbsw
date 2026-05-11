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

#include "routing/ErrorHandler.h"
#include "routing/RxAdapter.h"
#include "routing/RxAdapterTable.h"

#include <etl/array.h>
#include <etl/span.h>
#include <io/IReader.h>

#include <platform/estdint.h>

namespace routing
{
/**
 * Extracts the PDUs contained in the messages read from inputReader, adapting the IReader
 * interface, according to the provided RxAdapterTable.
 *
 * [TPARAMS_BEGIN]
 * \tparam MAX_ELEMENT_SIZE The maximum size of an extracted PDU.
 * [TPARAMS_END]
 */
template<size_t MAX_ELEMENT_SIZE>
class LegacyRxAdapter : public RxAdapter
{
public:
    // [PUBLIC_API_BEGIN]
    /**
     * Construct an LegacyRxAdapter with a pre-filled RxAdapterTable.
     */
    LegacyRxAdapter(
        ::io::IReader& inputReader, RxAdapterTable const& table, ErrorHandler errorHandler);

    /**
     * Construct an LegacyRxAdapter from the provided config byte array.
     */
    LegacyRxAdapter(
        ::io::IReader& inputReader, ::etl::span<uint8_t const> config, ErrorHandler errorHandler);

    /**
     * Return the MAX_ELEMENT_SIZE
     */
    size_t maxSize() const override;

    // [PUBLIC_API_END]

private:
    ::etl::array<uint8_t, MAX_ELEMENT_SIZE> _pduBuffer;
};

template<size_t MAX_ELEMENT_SIZE>
LegacyRxAdapter<MAX_ELEMENT_SIZE>::LegacyRxAdapter(
    ::io::IReader& inputReader, RxAdapterTable const& table, ErrorHandler const errorHandler)
: RxAdapter(inputReader, _pduBuffer, table, errorHandler)
{}

template<size_t MAX_ELEMENT_SIZE>
LegacyRxAdapter<MAX_ELEMENT_SIZE>::LegacyRxAdapter(
    ::io::IReader& inputReader,
    ::etl::span<uint8_t const> const config,
    ErrorHandler const errorHandler)
: RxAdapter(inputReader, _pduBuffer, config, errorHandler)
{}

template<size_t MAX_ELEMENT_SIZE>
size_t LegacyRxAdapter<MAX_ELEMENT_SIZE>::maxSize() const
{
    return MAX_ELEMENT_SIZE;
}

} // namespace routing
