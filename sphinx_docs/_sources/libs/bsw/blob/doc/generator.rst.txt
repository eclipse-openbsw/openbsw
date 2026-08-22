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

Regenerating the referenceApp headers
-------------------------------------

The script ``tools/blob/regenerate.sh`` regenerates all three generated headers.
It accepts positional arguments so it can be reused by any project that vendors
this repository:

.. code-block:: console

    tools/blob/regenerate.sh JSONL_FILE OUT_BLOB_DIR OUT_ROUTING_DIR

For the reference application, run from the **project root**:

.. code-block:: console

    # Inside the openbsw-development Docker container (deps pre-installed):
    tools/blob/regenerate.sh \
        executables/referenceApp/configuration/routing.jsonl \
        executables/referenceApp/configuration/include/blob \
        executables/referenceApp/configuration/include/routing

.. code-block:: console

    # On a host machine — set up a venv first (once):
    python3 -m venv tools/.venv
    tools/.venv/bin/pip install -r tools/blob/requirements.txt

    # Then run, pointing PYTHON at the venv interpreter:
    PYTHON=tools/.venv/bin/python3 tools/blob/regenerate.sh \
        executables/referenceApp/configuration/routing.jsonl \
        executables/referenceApp/configuration/include/blob \
        executables/referenceApp/configuration/include/routing

The script resolves all paths to absolute before changing directory internally,
so relative paths work as long as they are valid from the directory where you
invoke the script. It writes:

* ``OUT_BLOB_DIR/configuration.h`` — binary blob as a ``uint8_t`` array
* ``OUT_BLOB_DIR/ConfigType.h`` — ``ConfigType`` enum
* ``OUT_ROUTING_DIR/channelId.h`` — per-channel ID constants

After regeneration, review the hand-maintained ``constants.h`` alongside
``channelId.h`` if any channel IDs changed.

Manual equivalent
+++++++++++++++++

The script runs these three commands (from the ``tools`` directory, with the venv
active or the container's ``/opt/venv`` on ``PATH``):

.. code-block:: console

    cd tools
    JSONL=../executables/referenceApp/configuration/routing.jsonl
    OUT=../executables/referenceApp/configuration/include

    python3 -m blob binary -i "${JSONL}" -c blob.routing.table \
        | python3 -m blob header data -n CONFIGURATION_BLOB \
            -o "${OUT}/blob/configuration.h"

    python3 -m blob header config-type \
        -o "${OUT}/blob/ConfigType.h"

    python3 -m blob.routing header "${JSONL}" \
        -o "${OUT}/routing/channelId.h"
