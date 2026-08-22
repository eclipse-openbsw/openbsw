..
   *******************************************************************************
   Copyright (c) 2024 Accenture

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

transportRouterSimple
=====================

Overview
--------
The class ``TransportRouterSimple`` acts as an interface between transport
layers. It forwards transport messages coming from one transport layer to other.
It is also responsible to obtain message buffers to store the messages received
from the transport layers.