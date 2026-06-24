..
   *******************************************************************************
   Copyright (c) 2026 BMW AG

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

Generator
=========

The ``blob`` Python tool located in ``tools/blob`` generates blobs from a JSON Lines (``.jsonl``)
description of the desired configurations. It emits either the binary blob or generated C++ header
files for use in applications.

Usage
-----

Invoke the tool as a Python module from the ``tools`` directory so that the ``blob`` package can be
imported:

.. code-block:: console

    cd tools
    python -m blob <command> [options]

Commands
--------

``binary``
    Generates the binary blob from a ``.jsonl`` input and writes it to ``--output`` (or stdout).
    Configuration builders are selected with ``--config``, e.g. ``--config blob.routing``.

``pprint``
    Pretty-prints the blob structure as annotated, commented byte rows, such as those shown in
    :doc:`examples`. Useful for inspecting the layout described in :doc:`configuration`.

``header data``
    Emits a C++ header that embeds the blob as a ``uint8_t`` array (``--name`` controls the array
    name, default ``BLOB_DATA``).

``header config-type``
    Emits a C++ header containing the ``ConfigType`` enum used by the module
    (see :doc:`configuration`).

``header metadata``
    Emits a C++ header containing the ``Metadata`` enum derived from the ``meta`` entries in the
    input.

Each command accepts ``-i``/``--input`` (defaulting to stdin) and ``-o``/``--output``
(defaulting to stdout).

Example
-------

Generate a binary blob containing routing information and embed it into a C++ header:

.. code-block:: console

    cd tools
    python -m blob binary --config blob.routing -i routing.jsonl -o blob.bin
    python -m blob header data -i blob.bin -o BlobData.h

The generator computes the per-configuration CRC and ``0xFF`` padding automatically, so the
resulting blob passes the validation performed by ``load()`` and ``checkCrc()`` at runtime.

Routing helper
--------------

The ``blob.routing`` subpackage provides an auxiliary CLI for working with routing tables:

.. code-block:: console

    cd tools
    python -m blob.routing <sort|pprint|visualize|header> [options]

It can sort routings by channel ID, pretty-print them, render a Graphviz ``.dot`` visualization of
the routing graph, and generate a C++ header containing the routing channel IDs.

Commands for generating necessary blob files
--------------------------------------------

.. code-block:: console

    python -m blob binary -i routing.jsonl -c blob.routing.table | python -m blob header data -n CONFIGURATION_BLOB > include/blob/configuration.h
    python -m blob header config-type > include/blob/ConfigType.h
    python -m blob.routing header routing.jsonl > include/routing/channelId.h

This generates the configuration blob, the ``ConfigType`` enum header file, and the routing channel ID header file, which are required for the ``RoutingTable`` module to function correctly.
From the ``channelId`` file, one can construct the ``include/routing/constants.h`` file, which contains constants that are used in the routing module.