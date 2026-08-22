..
   *******************************************************************************
   Copyright (c) 2024 Accenture

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

.. _io_ireader:

io::IReader
===========

The interface ``IReader`` is the counterpart of :ref:`io_IWriter` and an abstraction to read
chunks of bytes with variable length from a data channel.

Public API
----------

.. sourceinclude:: include/io/IReader.h
   :start-after: PUBLICAPI_START
   :end-before: PUBLICAPI_END
   :dedent: 4

Usage of API
------------

The next sequence diagram visualizes the two step ``peek()`` and ``release()`` API. After a call to ``peek()``
returns a etl::span of data with size greater than zero, the user can consume this data. A call
to ``release()`` frees the data again, also invalidating it for the user.
**It must not be used** anymore after calling ``release()``.

.. uml::
    :scale: 100%

    actor SW
    SW -> IReader : auto d = peek()
    alt d.size() > 0 case
        SW -> SW  : consume d
        SW -> IReader : release()
    end

Example
+++++++

.. sourceinclude:: examples/MemoryQueueExample.cpp
   :start-after: EXAMPLE_BEGIN IReader
   :end-before: EXAMPLE_END IReader
