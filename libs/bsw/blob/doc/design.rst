..
   *******************************************************************************
   Copyright (c) 2026 BMW AG

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

Module Design
=============

The ``blob`` module is a small, allocation-free library for reading and validating the binary
gateway tables described in :doc:`configuration`. It never copies the underlying data;
instead it operates on ``::etl::span<uint8_t const>`` views into the memory region that holds the
blob.

Components
----------

``Header``
    Plain data structure describing the blob header (``version``, ``magic``, ``size``).
    ``checkHeader()`` validates the version and magic number and populates the declared data size.

``Blob``
    Wraps a validated blob span and exposes a forward (input) iterator over its configurations. The
    constructor calls ``checkHeader()`` and verifies that the memory region size matches
    ``HEADER_SIZE + size``; on failure it logs an error and yields an empty span. Iteration
    advances by each configuration's declared size and stops once the span is exhausted.

``Config``
    Represents a single configuration as a ``type`` and a ``data`` span. ``Config::from_bytes()``
    parses a configuration header (via ``loadConfigHeader()``), rejects ``UNKNOWN`` types and
    inconsistent sizes, and returns a span bounded to the configuration's extent.

``util``
    Free functions for higher-level access: ``load()`` performs full validation, ``config()`` looks
    up the first configuration of a given type, ``checkCrc()`` verifies a configuration's trailing
    CRC, and the ``loadColumn()`` template safely reinterprets a sized byte run as a typed column.

Loading and validation flow
---------------------------

The recommended entry point is ``load()``. It performs the full sequence of checks and returns a
span that is guaranteed to reference a structurally valid, CRC-verified blob (or an empty span on
failure):

.. uml::

    start
    :load(mem);
    partition checkHeader {
      if (mem.size >= HEADER_SIZE?) then (no)
        :return false;
      elseif (version valid?) then (no)
        :return false;
      elseif (magic valid?) then (no)
        :return false;
      else (yes)
        :return true;
      endif
    }
    if (checkHeader succeeded?) then (no)
      :return empty span;
      stop
    endif
    if (mem.size >= HEADER_SIZE + header.size?) then (no)
      :return empty span;
      stop
    endif
    :iterate configurations;
    repeat
      :checkCrc(config.data);
    repeat while (more configurations?)
    if (all CRCs valid?) then (no)
      :return empty span;
      stop
    endif
    :return blob span;
    stop

Design rationale
----------------

* **Zero-copy and allocation-free.** All access is span-based, making the module suitable for
  constrained embedded targets and safe to use on read-only memory such as flash.
* **Fail-safe.** Every parsing step is size-checked, and CRC validation guards against corruption.
  Invalid input collapses to an empty span rather than producing undefined behaviour.
* **Lazy iteration.** Configurations are decoded on demand by the ``Blob`` iterator, so the cost of
  parsing is paid only for the configurations that are actually inspected.