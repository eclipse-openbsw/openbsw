/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "someip/IServiceListener.h"
#include "someip/RpcChannel.h"

class ServiceListener : public ::someip::IServiceListener
{
    ::someip::RpcChannel& _rpcChannel;

public:
    explicit ServiceListener(::someip::RpcChannel& rpcChannel);

    void serviceStatusChanged(
        ::someip::ServiceDescription const& service, ServiceStatus status) override;
    void updateEventgroupDescription(::someip::ServiceDescription&, ServiceStatus) override;
    ::someip::MethodDetail const* getMethodDetail(uint16_t methodId) const override;
    ::someip::EventDetail const* getEventDetail(uint16_t eventId) const override;
};
