/********************************************************************************
 * Copyright (c) 2026 KPIT Technologies Ltd.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "middleware/bindings/someip/MemberBinding.h"
#include "someip/ServiceInfo.h"

#include <etl/vector.h>

#include <cstdint>

namespace middleware::bindings::someip
{

namespace internal
{

/**
 * The members of one service that are bound to SOME/IP, for its provided and its
 * consumed instance. Kept sorted by instance and member id, so a lookup is a binary
 * search. Works on storage its owner sizes.
 */
class MemberTable
{
public:
    explicit MemberTable(::etl::ivector<MemberBinding>& bindings);

    /**
     * A table too small for the service is a configuration error: a member
     * dropped here would surface only as "method not available" on the wire.
     * Inserting after equal keys keeps the first-bound member the one found.
     */
    void add(MemberBinding const& binding);

    /** The member \p memberId of \p instanceId, or nullptr if it is not bound. */
    MemberBinding const* find(uint16_t instanceId, uint16_t memberId) const;

    /** What the stack is told when a member of \p kind arrives as a method call. */
    static ::someip::MethodDetail methodDetailFor(uint16_t methodId, MemberBinding::Kind kind);

private:
    /** Instance in the high half, member in the low half. */
    static uint32_t keyOf(uint16_t instanceId, uint16_t memberId);

    static uint32_t keyOf(MemberBinding const& binding);

    ::etl::ivector<MemberBinding>& _bindings;
};

} // namespace internal

} // namespace middleware::bindings::someip
