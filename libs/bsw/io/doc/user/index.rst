..
   *******************************************************************************
   Copyright (c) 2024 Accenture

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

User Documentation
==================

Interfaces and data structures provided by ``io`` are described on the following pages.

Interfaces
----------

.. toctree::
   :hidden:

   iwriter
   ireader


.. csv-table::
   :widths: 20,80
   :width: 100%

   :ref:`io_IWriter`, "Generic interface to write into a data structure"
   :ref:`io_IReader`, "Generic interface to read from a data structure"

Data Structures
---------------

.. toctree::
   :hidden:

   join_reader
   forwarding_reader
   buffered_writer
   split_writer
   memory_queue
   variant_queue

.. csv-table::
   :widths: 20,80
   :width: 100%

   :ref:`io_JoinReader`, "Read from multiple readers"
   :ref:`io_ForwardingReader`, "Read from reader and forward data to writer"
   :ref:`io_BufferedWriter`, "Writer with an internal buffer"
   :ref:`io_SplitWriter`, "Write to multiple writers"
   :ref:`io_MemoryQueue`, "Single producer single consumer shared memory queue"
   :ref:`io_VariantQueue`, "(De)serialization mechanism to pass typed structs via ``MemoryQueue``"
