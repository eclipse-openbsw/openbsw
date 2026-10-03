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

The module entry point is ``load()``. It validates a memory region that should contain a blob and
returns the validated span to pass to consuming modules. ``Blob`` can also be instantiated directly
to iterate over configurations, but consumers should normally use the span returned by ``load()`` and
construct a ``Blob`` iterator only when they need to inspect configurations.

Workflow
--------

The ``blob`` module holds no state and owns no resources, so there is no separate
de-initialization step. A typical integration is:

1. **Locate the blob.** Obtain an ``::etl::span<uint8_t const>`` over the memory region that holds
   the blob. No copy into RAM is required.
2. **Validate once at startup.** Call ``load()`` a single time. It verifies the header
   (version + magic), checks that the region is at least ``HEADER_SIZE + size`` bytes, and
   re-computes the CRC-32 of *every* configuration. On success it returns a span that is guaranteed
   to reference a structurally valid, CRC-verified blob; on any failure it logs and returns an empty
   span. Treat an empty span as a hard configuration error.
3. **Access configurations.** Pass the validated span to the consuming modules. Use
   ``config(blob, type)`` to fetch the first configuration of a given type, or iterate with
   ``Blob`` when several are needed. Because the blob is read-only, these accessors are reentrant
   and may be called from any context.
4. **De-initialization.** None required — simply stop referencing the span. If the underlying
   memory can be unmapped or overwritten, ensure no consumer still holds a span into it.

Performance and efficiency
--------------------------

* ``load()`` walks the whole blob and re-computes every CRC, so call it once during startup and keep
  the returned span for later access.
* Access is span-based and allocation-free; the module never copies the blob data.
* ``config()`` starts iteration from the beginning on each call. If several configurations are
  needed, iterate once with ``Blob`` instead of doing repeated lookups.
