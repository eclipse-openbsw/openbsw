/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

// include all header files to get complete code coverage

// someip
#include <someip/ISomeIpStack.h>
#include <someip/SomeIpStack.h>
#include <someip/init.h>

// diagnosis
#include <someip/Statistics.h>
#include <someip/StatisticsEvaluator.h>

// message
#include <someip/SomeIpMessage.h>

// network
#include <someip/INetwork.h>
#include <someip/INetworkListener.h>
#include <someip/Network.h>
#include <someip/NetworkChannel.h>
#include <someip/NetworkConfig.h>
#include <someip/NetworkResource.h>

// network/tcp
#include <someip/TcpConfig.h>
#include <someip/TcpProxy.h>
#include <someip/TcpServer.h>
#include <someip/UdpConfig.h>
#include <someip/UdpProxy.h>

// rpc
#include <someip/IRpcChannel.h>
#include <someip/IRpcHandler.h>
#include <someip/IRpcReceiver.h>
#include <someip/IRpcSender.h>
#include <someip/RequestContext.h>
#include <someip/RpcBaseClient.h>
#include <someip/RpcChannel.h>
#include <someip/RpcClosure.h>
#include <someip/RpcHandler.h>
#include <someip/RpcReceiver.h>
#include <someip/ServiceHandler.h>

// rpc/events
#include <someip/BufferedEventSender.h>
#include <someip/EventTransceiver.h>
#include <someip/IEventListener.h>
#include <someip/IEventProvider.h>
#include <someip/IEventReceiver.h>
#include <someip/IEventSender.h>

// sd
#include <someip/IProvidedServiceListener.h>
#include <someip/IServiceAnnouncer.h>
#include <someip/IServiceRegistry.h>
#include <someip/ISubscriptionManager.h>
#include <someip/ProvidedService.h>
#include <someip/SdConstants.h>
#include <someip/SdEndpoint.h>
#include <someip/ServiceDescription.h>
#include <someip/ServiceManager.h>
#include <someip/ServiceQuery.h>
#include <someip/SubscribedEventGroup.h>
#include <someip/SubscriptionEndpoint.h>
#include <someip/SubscriptionManager.h>

// sd/local
#include <someip/RpcServiceRegistry.h>
#include <someip/RpcSomeIpStack.h>

// sd/remote
#include <someip/ISdMessageParser.h>
#include <someip/IServiceTrackerListener.h>
#include <someip/QueryManager.h>
#include <someip/RebootTracker.h>
#include <someip/RebootTrackerEndpoint.h>
#include <someip/SdMessageBuilder.h>
#include <someip/SdMessageParser.h>
#include <someip/SdOptionParser.h>
#include <someip/SdOptions.h>
#include <someip/SdReceiver.h>
#include <someip/SdServiceRegistry.h>
#include <someip/SdSomeIpStack.h>
#include <someip/ServiceAnnouncer.h>
#include <someip/ServiceAnnouncerTask.h>
#include <someip/ServiceTracker.h>
#include <someip/SessionManager.h>

// serialization
#include <someip/EnumSerializable.h>
#include <someip/EtlArray.h>
#include <someip/EtlString.h>
#include <someip/EtlVector.h>
#include <someip/ISomeIpSerializable.h>
#include <someip/LengthHelper.h>
#include <someip/SomeIpParser.h>
#include <someip/SomeIpSerializer.h>
#include <someip/SomeIpStreamer.h>
#include <someip/UnionBase.h>

// types
#include <someip/CodingValidation.h>
#include <someip/PrimitiveTypes.h>
#include <someip/SomeIpConstants.h>
#include <someip/SomeIpStack.h>
#include <someip/StringEncoding.h>
