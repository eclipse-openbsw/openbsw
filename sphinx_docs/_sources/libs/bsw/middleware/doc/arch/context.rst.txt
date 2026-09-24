..
   *******************************************************************************
   Copyright (c) 2026 BMW AG

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

Context View
============

The middleware interacts with the following components:

Async / RTOS
    Schedules the cyclic runnables of the middleware.

BSW Integration
    Provides a mutex API, logging, and timing services.

Deployment model
    The model defines application interfaces transferred over the middleware.
    It also provides deployment information that allocates service instances to
    middleware instances.

Shared Memory Queue
    Middleware instances communicate with each other over shared memory.
    This is the default path for inter-process communication.

User Application
    The user application uses the middleware to communicate with other service
    providers and consumers.
    It can be a service consumer or a service provider.

The context is also depicted in the diagram below.
If unlabelled, the arrows are usage relationships.

.. uml:: middleware_context.puml
    :caption: Middleware context
