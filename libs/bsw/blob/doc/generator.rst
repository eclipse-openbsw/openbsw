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

Routing headers can be generated in either of two ways:

* Bazel builds can use the rule in ``tools/blob/routing.bzl``. This makes code
    generation hermetic and tracks generated files through the Bazel build graph.
* CMake and other non-Bazel workflows can use the build-system-independent
    scripts. The project-specific ``regenerate.sh`` wrappers regenerate the
    checked-in headers for the reference application and tests. The shared
    ``tools/blob/regenerate.sh`` script accepts an input file and output
    directories for other integrations.

The standalone scripts require Python 3 with venv support. They install the
locked dependency from ``tools/blob/requirements.lock`` into a user cache when
it is not already available. The generated files are not post-processed, so the
standalone scripts and Bazel rule produce identical output.

For example, regenerate the reference application headers from the repository
root:

.. code-block:: console

    executables/referenceApp/configuration/regenerate.sh

This command invokes the reference application's standalone wrapper. For other
configurations, invoke the shared standalone script with the input and output
directories:

.. code-block:: console

    tools/blob/regenerate.sh INPUT.jsonl OUTPUT_BLOB_DIR OUTPUT_ROUTING_DIR

The following ``BUILD.bazel`` example creates a target that exposes generated
headers to Bazel C++ consumers:

.. code-block:: python

    load("//tools/blob:routing.bzl", "generate_routing")

    generate_routing(
        name = "routing_configuration",
        jsonl = "routing.jsonl",
    )

The rule provides these generated headers:

* ``blob/configuration.h`` — binary blob as a ``uint8_t`` array
* ``blob/ConfigType.h`` — ``ConfigType`` enum
* ``routing/channelId.h`` — per-channel ID constants
* ``routing/constants.h`` — generated channel-count constants

Both the Bazel rule and standalone script invoke the same deterministic Python
generator, so generation logic is shared rather than duplicated.
