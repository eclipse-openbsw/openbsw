..
   *******************************************************************************
   Copyright (c) 2024 Accenture

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

CMake
=====

We use `cmake-format <https://github.com/cheshirekow/cmake_format>`_ to format CMake files.

Usage
-----

Formatting a file:

.. code-block:: bash

    cmake-format -style=file -i <file_to_check>

Formatting several files at once:

.. code-block:: bash

    cmake-format -i $(find . -name CMakeLists.txt | sed '/3rdparty\/.*\/CMakeLists\.txt/d')

*-i* edits the files in-place.

cmake-format is also called by *treefmt*, see :ref:`automatic_formatting`.
So the easiest way to format all CMake files is using treefmt:

.. code-block:: bash

    treefmt

Configuration
-------------

Configuration is provided in the `.cmake-format` file:

.. sourceinclude:: ../../../../.cmake-format
   :language: python
