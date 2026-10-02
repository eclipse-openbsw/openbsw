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

#include <etl/span.h>
#include <cstddef>
#include <cstdint>

namespace middleware::core
{

/**
 * \brief Evaluates a PDU snapshot; implemented and owned by the application.
 *
 * Results are exposed by the concrete implementation, not by this interface.
 */
class IQualifier
{
public:
    IQualifier()                             = default;
    virtual ~IQualifier()                    = default;
    IQualifier(IQualifier const&)            = default;
    IQualifier& operator=(IQualifier const&) = default;
    IQualifier(IQualifier&&)                 = default;
    IQualifier& operator=(IQualifier&&)      = default;

    /**
     * \param seqNumber Sequence number of the snapshot.
     * \param rawData Raw bytes of the snapshot.
     */
    virtual void qualify(uintptr_t const seqNumber, etl::span<uint8_t const> rawData) = 0;
};

} // namespace middleware::core
