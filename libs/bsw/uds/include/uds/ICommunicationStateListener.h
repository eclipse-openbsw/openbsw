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
#include <etl/uncopyable.h>

namespace uds
{
class ICommunicationStateListener
: public ::etl::bidirectional_link<0>
, public ::etl::uncopyable
{
public:
    enum CommunicationState
    {
        ENABLE_NORMAL_MESSAGE_TRANSMISSION,
        DISABLE_NORMAL_MESSAGE_TRANSMISSION,
        ENABLE_REC_DISABLE_NORMAL_MESSAGE_SEND_TRANSMISSION,
        ENABLE_NN_MESSAGE_TRANSMISSION,
        DISABLE_NM_MESSAGE_TRANSMISSION,
        ENABLE_REC_DISABLE_NM_SEND_TRANSMISSION,
        ENABLE_ALL_MESSAGE_TRANSMISSION,
        DISABLE_ALL_MESSAGE_TRANSMISSION,
        ENABLE_REC_DISABLE_ALL_SEND_TRANSMISSION,
        DISABLE_REC_ENABLE_NORMAL_MESSAGE_SEND_TRANSMISSION,
        DISABLE_REC_ENABLE_NM_SEND_TRANSMISSION,
        DISABLE_REC_ENABLE_ALL_SEND_TRANSMISSION
    };

    ICommunicationStateListener() {}

    virtual void communicationStateChanged(CommunicationState newState) = 0;
};

} // namespace uds
