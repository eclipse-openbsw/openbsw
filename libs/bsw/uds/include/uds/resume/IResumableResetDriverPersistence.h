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

#include "transport/TransportMessage.h"

namespace uds
{
class IResumableResetDriverPersistence
{
public:
    IResumableResetDriverPersistence() = default;

    IResumableResetDriverPersistence(IResumableResetDriverPersistence const&)            = delete;
    IResumableResetDriverPersistence& operator=(IResumableResetDriverPersistence const&) = delete;

    /**
     * Check whether reset is pending and should be resumed.
     * \param message reference to message that should be filled if existing
     * \return true if a message has been stored. Only in this case the me
     */
    virtual bool readRequest(::transport::TransportMessage& message) const = 0;

    /**
     * Write the request to resume.
     * \param message message to write
     */
    virtual void writeRequest(::transport::TransportMessage const& message) = 0;

    /**
     * Clear any stored data.
     */
    virtual void clear() = 0;
};

} // namespace uds
