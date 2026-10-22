/********************************************************************************
 * Copyright (c) 2026 KPIT Technologies Ltd.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "middleware/bindings/someip/MemberTable.h"

#include "someip/SomeIpConstants.h"

#include <etl/algorithm.h>
#include <etl/error_handler.h>

namespace middleware::bindings::someip
{

namespace internal
{

MemberTable::MemberTable(::etl::ivector<MemberBinding>& bindings) : _bindings(bindings) {}

void MemberTable::add(MemberBinding const& binding)
{
    if (_bindings.full())
    {
        ETL_ASSERT_FAIL(
            ETL_ERROR_GENERIC("SomeIpServiceEndpoint: binding table full, raise MaxMembers"));
        return;
    }
    auto const position = ::etl::upper_bound(
        _bindings.begin(),
        _bindings.end(),
        keyOf(binding),
        [](uint32_t const key, MemberBinding const& entry) { return key < keyOf(entry); });
    _bindings.insert(position, binding);
}

MemberBinding const* MemberTable::find(uint16_t const instanceId, uint16_t const memberId) const
{
    uint32_t const key = keyOf(instanceId, memberId);
    auto const it      = ::etl::lower_bound(
        _bindings.begin(),
        _bindings.end(),
        key,
        [](MemberBinding const& entry, uint32_t const k) { return keyOf(entry) < k; });
    return ((it != _bindings.end()) && (keyOf(*it) == key)) ? &(*it) : nullptr;
}

::someip::MethodDetail
MemberTable::methodDetailFor(uint16_t const methodId, MemberBinding::Kind const kind)
{
    ::someip::MethodDetail detail{};
    detail.methodId = methodId;
    switch (kind)
    {
        case MemberBinding::Kind::FireAndForget:
            detail.methodType   = ::someip::SomeIpMethodType::METHOD_REQUEST_ONLY;
            detail.callSemantic = ::someip::SomeIpCallSemantic::SEMANTIC_FIRE_AND_FORGET;
            break;
        case MemberBinding::Kind::RequestResponse:
            detail.methodType   = ::someip::SomeIpMethodType::METHOD_REQUEST_RESPONSE;
            detail.callSemantic = ::someip::SomeIpCallSemantic::SEMANTIC_REQUEST_RESPONSE;
            break;
        case MemberBinding::Kind::RequestOnly:
            detail.methodType   = ::someip::SomeIpMethodType::METHOD_REQUEST_ONLY;
            detail.callSemantic = ::someip::SomeIpCallSemantic::SEMANTIC_REQUEST_RESPONSE;
            break;
        case MemberBinding::Kind::ResponseOnly:
            detail.methodType   = ::someip::SomeIpMethodType::METHOD_RESPONSE_ONLY;
            detail.callSemantic = ::someip::SomeIpCallSemantic::SEMANTIC_REQUEST_RESPONSE;
            break;
        case MemberBinding::Kind::Event:
        default:                         break;
    }
    return detail;
}

uint32_t MemberTable::keyOf(uint16_t const instanceId, uint16_t const memberId)
{
    return (static_cast<uint32_t>(instanceId) << 16U) | static_cast<uint32_t>(memberId);
}

uint32_t MemberTable::keyOf(MemberBinding const& binding)
{
    return keyOf(binding.instanceId, binding.memberId);
}

} // namespace internal

} // namespace middleware::bindings::someip
