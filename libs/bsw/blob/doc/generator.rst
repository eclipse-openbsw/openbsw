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

Install the Python dependency before using the tool. Inside the
``openbsw-development`` Docker container it is already available. On a host machine,
create a venv:

.. code-block:: console

    python3 -m venv tools/.venv
    tools/.venv/bin/pip install -r tools/blob/requirements.txt
    # Activate before running any blob command:
    source tools/.venv/bin/activate

The currently required package is ``crc==7.1.0`` (see ``tools/blob/requirements.txt``).

Input format
------------

Input is one JSON object per line. The minimum useful input for routing usually contains
``channel`` and ``routing`` entries. A minimal example is shown below.

.. code-block:: json

    {"type": "channel", "value": {"name": "CAN0", "type": "can", "id": 0}}
    {"type": "channel", "value": {"name": "ETH0", "type": "pdu_transport"}}
    {"type": "routing", "value": {"input": {"channel-name": "CAN0", "message-id": 256, "offset": 0, "pdu-length": 8}, "output": [{"channel-name": "ETH0", "message-id": 2304, "offset": 0, "pdu-length": 8}]}}

Usage
-----

Invoke the tool as a Python module from the ``tools`` directory (so that the
``blob`` package is importable) with the venv active:

.. code-block:: console

    cd tools
    python3 -m blob <command> [options]

Commands
--------

``binary``
    Generates the binary blob from a ``.jsonl`` input and writes it to ``--output`` (or stdout).
    Configuration builders are selected with ``--config``, e.g. ``--config blob.routing``.
    CLI: ``blob binary -i INPUT.jsonl [-c MODULE ...] [-o OUTPUT.bin]``

``pprint``
    Pretty-prints the blob structure as annotated, commented byte rows, such as those shown in
    :doc:`examples`. Useful for inspecting the layout described in :doc:`configuration`.
    CLI: ``blob pprint -i INPUT.jsonl [-c MODULE ...]``

``header data``
    Emits a C++ header that embeds the blob as a ``uint8_t`` array (``--name`` controls the array
    name, default ``BLOB_DATA``).
    CLI: ``blob header data -i INPUT.bin [-n NAME] [-o OUTPUT.h]``

``header config-type``
    Emits a C++ header containing the ``ConfigType`` enum used by the module
    (see :doc:`configuration`).
    CLI: ``blob header config-type [-n NAME] [-o OUTPUT.h]``

``header metadata``
    Emits a C++ header containing the ``Metadata`` enum derived from the ``meta`` entries in the
    input.
    CLI: ``blob header metadata -i INPUT.jsonl [-n NAME] [-o OUTPUT.h]``

Example
-------

Generate a binary blob containing routing information and embed it into a C++ header:

.. code-block:: console

    cd tools
    python3 -m blob binary --config blob.routing -i routing.jsonl -o blob.bin
    python3 -m blob header data -i blob.bin -o BlobData.h

The generator computes the per-configuration CRC and ``0xFF`` padding automatically, so the
resulting blob passes the validation performed by ``load()`` and ``checkCrc()`` at runtime.

Routing helper
--------------

The ``blob.routing`` subpackage provides an auxiliary CLI for working with routing tables:

.. code-block:: console

    cd tools
    python3 -m blob.routing <sort|pprint|visualize|header> [options]

It can sort routings by channel ID, pretty-print them, render a Graphviz ``.dot`` visualization of
the routing graph, and generate a C++ header containing the routing channel IDs.

Using the Bazel generator rule
------------------------------

The supported generation path is the Bazel rule in ``tools/blob/routing.bzl``.
It keeps code generation hermetic and tracked through the same build graph as the
rest of the project, so there is no separate shell wrapper to maintain.

.. code-block:: python

    load("//tools/blob:routing.bzl", "generate_routing")

    generate_routing(
        name = "routing_configuration",
        jsonl = "routing.jsonl",
    )

This rule emits the same generated headers as the old script:

* ``blob/configuration.h`` — binary blob as a ``uint8_t`` array
* ``blob/ConfigType.h`` — ``ConfigType`` enum
* ``routing/channelId.h`` — per-channel ID constants
* ``routing/constants.h`` — generated channel-count constants

The generated outputs are then consumed through the Bazel target dependency graph,
which avoids duplicating generation logic in repo-local shell scripts.
