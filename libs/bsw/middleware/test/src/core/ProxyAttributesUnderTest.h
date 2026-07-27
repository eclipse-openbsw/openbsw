/********************************************************************************
 * Copyright (c) 2024 Accenture, 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "middleware/core/ProxyAttribute.h"
#include "middleware/core/ProxyBase.h"
#include "middleware/core/types.h"
#include "middleware/rpc/ProxyMethod.h"
#include "mock/ProxyMock.h"

namespace middleware::core::test
{

namespace internal
{
constexpr uint16_t get_id   = 0;
constexpr uint16_t set_id   = 1U;
constexpr uint16_t event_id = 2U;
} // namespace internal

static constexpr uint32_t TIMEOUT_VALUE = 5000U;
static constexpr uint32_t REQUEST_LIMIT = 1U;

using ArgType        = uint32_t;
using RequestResult  = ::etl::expected<uint16_t, HRESULT>;
using GetterPolicy   = rpc::MethodTraits<void, ArgType, TIMEOUT_VALUE>;
using SetterPolicy   = rpc::MethodTraits<ArgType, ArgType, TIMEOUT_VALUE>;
using NoSetterPolicy = void;

using FullyFeaturedAttribute
    = ProxyAttribute<ProxyMock, GetterPolicy, SetterPolicy, AttributeType::FullyFeatured, ArgType>;

using FullyFeaturedSetAsMethodAttribute = ProxyAttribute<
    ProxyMock,
    GetterPolicy,
    SetterPolicy,
    AttributeType::FullyFeatured_SetAsMethod,
    ArgType>;

using NoSubscriptionsAttribute = ProxyAttribute<
    ProxyMock,
    GetterPolicy,
    SetterPolicy,
    AttributeType::NoSubscriptions,
    ArgType>;

using NoSubscriptionsSetAsMethodAttribute = ProxyAttribute<
    ProxyMock,
    GetterPolicy,
    SetterPolicy,
    AttributeType::NoSubscriptions_SetAsMethod,
    ArgType>;

using ReadOnlyAttribute
    = ProxyAttribute<ProxyMock, GetterPolicy, NoSetterPolicy, AttributeType::ReadOnly, ArgType>;

using ReadOnlyNoSubscriptionAttribute = ProxyAttribute<
    ProxyMock,
    GetterPolicy,
    NoSetterPolicy,
    AttributeType::ReadOnly_NoSubscription,
    ArgType>;

struct DerivedFullyFeaturedAttribute final : public FullyFeaturedAttribute
{
    static constexpr middleware::core::AttributeType TYPE
        = middleware::core::AttributeType::FullyFeatured;
    using AttributeType          = ArgType;
    using Base                   = FullyFeaturedAttribute;
    using OnFieldChangedCallback = Base::OnFieldChangedCallback;

    explicit DerivedFullyFeaturedAttribute(ProxyBase& proxy) : Base(proxy) {}
};

struct DerivedFullyFeaturedSetAsMethodAttribute final : public FullyFeaturedSetAsMethodAttribute
{
    static constexpr middleware::core::AttributeType TYPE
        = middleware::core::AttributeType::FullyFeatured_SetAsMethod;
    using AttributeType          = ArgType;
    using Base                   = FullyFeaturedSetAsMethodAttribute;
    using OnFieldChangedCallback = Base::OnFieldChangedCallback;

    explicit DerivedFullyFeaturedSetAsMethodAttribute(ProxyBase& proxy) : Base(proxy) {}
};

struct DerivedNoSubscriptionsAttribute final : public NoSubscriptionsAttribute
{
    static constexpr middleware::core::AttributeType TYPE
        = middleware::core::AttributeType::NoSubscriptions;
    using AttributeType = ArgType;
    using Base          = NoSubscriptionsAttribute;

    explicit DerivedNoSubscriptionsAttribute(ProxyBase& proxy) : Base(proxy) {}
};

struct DerivedNoSubscriptionsSetAsMethodAttribute final : public NoSubscriptionsSetAsMethodAttribute
{
    static constexpr middleware::core::AttributeType TYPE
        = middleware::core::AttributeType::NoSubscriptions_SetAsMethod;
    using AttributeType = ArgType;
    using Base          = NoSubscriptionsSetAsMethodAttribute;

    explicit DerivedNoSubscriptionsSetAsMethodAttribute(ProxyBase& proxy) : Base(proxy) {}
};

struct DerivedReadOnlyAttribute final : public ReadOnlyAttribute
{
    static constexpr middleware::core::AttributeType TYPE
        = middleware::core::AttributeType::ReadOnly;
    using AttributeType          = ArgType;
    using Base                   = ReadOnlyAttribute;
    using OnFieldChangedCallback = Base::OnFieldChangedCallback;

    explicit DerivedReadOnlyAttribute(ProxyBase& proxy) : Base(proxy) {}
};

struct DerivedReadOnlyNoSubscriptionAttribute final : public ReadOnlyNoSubscriptionAttribute
{
    static constexpr middleware::core::AttributeType TYPE
        = middleware::core::AttributeType::ReadOnly_NoSubscription;
    using AttributeType = ArgType;
    using Base          = ReadOnlyNoSubscriptionAttribute;

    explicit DerivedReadOnlyNoSubscriptionAttribute(ProxyBase& proxy) : Base(proxy) {}
};

} // namespace middleware::core::test
