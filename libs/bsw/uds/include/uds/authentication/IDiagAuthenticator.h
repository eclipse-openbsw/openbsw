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

#include "uds/DiagReturnCode.h"

#include <etl/uncopyable.h>

#include <platform/estdint.h>

namespace uds
{
class IDiagAuthenticator : public ::etl::uncopyable
{
protected:
    IDiagAuthenticator() {}

public:
#ifdef UNIT_TEST
    virtual ~IDiagAuthenticator() {}
#endif

    /**
     * Checks if an entity is authenticated
     * \param address Address identifying the entity
     * \return Authentication status
     *          - true if entity is authenticated
     *          - false if entity is not authenticated
     */
    virtual bool isAuthenticated(uint16_t address) const = 0;

    /**
     * Return the DiagReturnCode to use when sending a negative response due to an entity not being
     * authenticated
     */
    virtual DiagReturnCode::Type getNotAuthenticatedReturnCode() const = 0;
};

} // namespace uds
