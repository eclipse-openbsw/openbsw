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

#include <etl/intrusive_list.h>

#include <platform/estdint.h>

namespace uds
{
class DiagSession;

/**
 * Interface for listeners to change of diagnosis session
 *
 *
 * \see IDiagSessionManager
 */
class IDiagSessionChangedListener : public ::etl::bidirectional_link<0>
{
public:
    virtual void diagSessionChanged(DiagSession const& session) = 0;
    virtual void diagSessionResponseSent(uint8_t responseCode)  = 0;
};

} // namespace uds
