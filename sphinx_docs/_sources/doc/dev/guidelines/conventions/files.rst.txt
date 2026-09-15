..
   *******************************************************************************
   Copyright (c) 2024 Accenture

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

Files
=====

.. toctree::
   :maxdepth: 2
   :caption: Contents:

Including Header Files
----------------------

:rule:`FILE-000` The style used to specify an include shall reflect if it's a system include or user defined file.

- All C and C++ standard library headers as well as all etl headers shall use angle brackets ``<>``.
- All other includes shall use quotes ``""``.

Standard Headers
----------------

- See `C++ Standard Library headers <https://en.cppreference.com/w/cpp/header>`_ for details.
  Note that not all includes make sense in embedded code. Some parts are replaced by the
  ``etl`` library.
- :rule:`FILE-010` If a file needs to be compiled for C++ and C, use the C headers.
