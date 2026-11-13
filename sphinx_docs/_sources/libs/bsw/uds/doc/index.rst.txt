..
   *******************************************************************************
   Copyright (c) 2024 Accenture

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

.. _uds:

uds - Unified Diagnostic Services
=================================

An implementation of the ISO-14229 specification.

Unified Diagnostic Services (UDS) is a protocol that allows diagnostic systems
to communicate with Electronic Control Units (ECUs) in vehicles. It is used to
diagnose errors, debug and reprogram ECUs.

Bounds checks
-------------

The clang-tidy fragments in ``include/`` and ``src/`` inherit the repository
configuration and enable ``cppcoreguidelines-avoid-c-arrays``,
``cppcoreguidelines-pro-bounds-array-to-pointer-decay`` and
``cppcoreguidelines-pro-bounds-pointer-arithmetic`` for production code.
These fragments do not enable the checks for tests or mocks.

Buffer parameters retain their pointer-and-length interfaces. Fixed-size storage
uses ``::etl::array``; implementations extending ``SecurityAccess`` can use
``fSeed.data()`` and ``fTesterId.data()`` when passing those buffers to pointer-based
interfaces.

.. toctree::
    :maxdepth: 1
    :glob:

    */index
