..
   *******************************************************************************
   Copyright (c) 2026 BMW AG

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

Integration
===========

The module defines the API ``load()``, which converts a pointer in memory to a span of data, after
sufficiently checking that this data is that of a blob.

Alternatively, the ``Blob`` class can be instantiated with a blob span, so as to neatly iterate over
configurations using the class's custom iterator implementation. However, it is recommended that the
blob span returned by ``load()`` be used for any further processing in a given module, and that the
``Blob`` iterator be constructed only on demand.