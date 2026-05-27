/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "ethernet/EthernetLogger.h"
#include "ip/IPAddress.h"
#include "ip/IPEndpoint.h"
#include "ip/NetworkInterfaceConfig.h"
#include "ip/to_str.h"
#include "tcp/IDataListener.h"
#include "tcp/IDataSendNotificationListener.h"
#include "tcp/TcpLogger.h"
#include "tcp/socket/AbstractServerSocket.h"
#include "tcp/socket/AbstractSocket.h"
#include "tcp/socket/ISocketProvidingConnectionListener.h"
#include "tcp/util/BandwidthTestSocket.h"
#include "tcp/util/LoopbackTestServer.h"
#include "tcp/util/TcpIperf2Server.h"
#include "udp/DatagramPacket.h"
#include "udp/IDataListener.h"
#include "udp/IDataSentListener.h"
#include "udp/UdpLogger.h"
#include "udp/socket/AbstractDatagramSocket.h"
#include "udp/util/UdpEchoTestServer.h"
#include "udp/util/UdpIperf2Server.h"

#include <gmock/gmock.h>

namespace
{
using namespace ::testing;

TEST(IncludeTest, TestIncludes) {}
} // anonymous namespace
