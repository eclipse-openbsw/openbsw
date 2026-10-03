..
   *******************************************************************************
   Copyright (c) 2026 BMW AG

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

Request/Response
================

This unit manages asynchronous request tracking and timeout handling for
middleware method and attribute-get flows.

It centralizes future-slot allocation, request identifiers, callback dispatch,
and timeout state management.

Class Diagram
-------------

.. uml:: request_response.puml
