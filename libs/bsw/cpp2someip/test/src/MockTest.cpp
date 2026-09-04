/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

// include all mocks to see if we have compile errors

#include "someip/BufferedEventSender.h"
#include "someip/CallDoneClosureMock.h"
#include "someip/EventListenerMock.h"
#include "someip/EventProviderMock.h"
#include "someip/EventReceiverMock.h"
#include "someip/EventSenderMock.h"
#include "someip/EventTransceiverMock.h"
#include "someip/NetworkListenerMock.h"
#include "someip/NetworkMock.h"
#include "someip/NetworkResourceMock.h"
#include "someip/RpcChannelMock.h"
#include "someip/RpcHandlerMock.h"
#include "someip/RpcReceiverMock.h"
#include "someip/RpcSenderMock.h"
#include "someip/SdMessageParserMock.h"
#include "someip/ServiceAnnouncerMock.h"
#include "someip/ServiceHandlerMock.h"
#include "someip/ServiceListenerMock.h"
#include "someip/ServiceRegistryMock.h"
#include "someip/SomeIpSerializableMock.h"
#include "someip/SomeIpStackMock.h"
#include "someip/SubscriptionManagerMock.h"
#include "someip/TcpProxyMock.h"
#include "someip/TpTransceiverMock.h"

#include <tcp/socket/AbstractSocketMock.h>

namespace
{
using namespace ::testing;

/**
 * Test declaration of TCP mock.
 */
TEST(Mock, declare_tcp_mock)
{
    ::tcp::AbstractSocketMock socket;
    StrictMock<::someip::TcpProxyMock> _tcpProxyMock(socket);
}

/**
 * Test declaration of network mocks.
 */
TEST(Mock, declare_network_mocks)
{
    StrictMock<::someip::NetworkListenerMock> _networkListenerMock1;
    StrictMock<::someip::NetworkMock> _networkMock;
    StrictMock<::someip::NetworkResourceMock> _networkResourceMock;
}

/**
 * Test declaration of RPC event mocks.
 */
TEST(Mock, declare_rpc_events_mocks)
{
    StrictMock<::someip::EventListenerMock> _eventListenerMock;
    StrictMock<::someip::EventReceiverMock> _eventReceiverMock;
    StrictMock<::someip::EventSenderMock> _eventSenderMock;

    StrictMock<::someip::NetworkMock> _networkMock;
    StrictMock<::someip::TpTransceiverMock> _tpTransceiverMock;
    async::ContextType _ethernetContext{0U};
    ::someip::declare::BufferedEventSender<1> _eventSender(
        _networkMock, _ethernetContext, _tpTransceiverMock);
    StrictMock<::someip::SubscriptionManagerMock> _subscriptionManagerMock;
    StrictMock<::someip::ServiceRegistryMock> _serviceRegistryMock;

    StrictMock<::someip::EventTransceiverMock> _eventTransceiverMock;
    StrictMock<::someip::EventProviderMock> _eventProviderMock;
}

/**
 * Test declaration of RPC mocks.
 */
TEST(Mock, declare_rpc_mocks)
{
    StrictMock<::someip::CallDoneClosureMock> _callDoneClosureMock;
    StrictMock<::someip::RpcChannelMock> _rpcChannelMock;
    StrictMock<::someip::RpcHandlerMock> _rpcHandlerMock;
    StrictMock<::someip::RpcReceiverMock> _rpcReceiverMock;
    StrictMock<::someip::RpcSenderMock> _rpcSenderMock;
    StrictMock<::someip::ServiceHandlerMock<5U>> _serviceHandlerMock;
}

/**
 * Test declaration of SOME/IP stack mock.
 */
TEST(Mock, declare_someip_stack_mock) { StrictMock<::someip::SomeIpStackMock> _someIpStackMock; }

/**
 * Test declaration of SD remote mock.
 */
TEST(Mock, declare_sd_remote_mock)
{
    StrictMock<::someip::SdMessageParserMock> _sdMessageParserMock;
}

/**
 * Test declaration of SD mocks.
 */
TEST(Mock, declare_sd_mocks)
{
    StrictMock<::someip::ServiceAnnouncerMock> _serviceAnnouncerMock;
    StrictMock<::someip::ServiceListenerMock> _serviceListenerMock;
    StrictMock<::someip::SubscriptionManagerMock> _subscriptionManagerMock;
    StrictMock<::someip::ServiceRegistryMock> _serviceRegistryMock;
}

/**
 * Test declaration of serialization mock.
 */
TEST(Mock, declare_serialization_mock)
{
    StrictMock<::someip::SomeIpSerializableMock> _serializableMock;
}
} // anonymous namespace
