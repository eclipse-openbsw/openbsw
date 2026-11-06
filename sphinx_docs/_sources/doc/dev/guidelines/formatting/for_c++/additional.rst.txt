..
   *******************************************************************************
   Copyright (c) 2024 Accenture

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

.. _additional_formatting_rules:

Additional Rules
================

.. _cf_include_guard:

Include Guard
-------------

Use ``#pragma once`` at the top of every header file to prevent multiple inclusion.
This is simpler and less error-prone than UUID-based ``#ifndef`` guards.

.. code-block:: cpp
    :caption: good example

    #pragma once

.. code-block:: cpp
    :caption: bad examples

    #ifndef GUARD_C73C2C69_6A11_4490_93AA_73A40D5F62F9
    #define GUARD_C73C2C69_6A11_4490_93AA_73A40D5F62F9
    // ...
    #endif // GUARD_C73C2C69_6A11_4490_93AA_73A40D5F62F9

    #ifndef CANTRANSCEIVER_H_
    #define CANTRANSCEIVER_H_
    // ...
    #endif /* CANTRANSCEIVER_H_ */


.. _cf_copyright_disclaimer:

Copyright Disclaimer
--------------------

Every source file must contain an Apache-2.0 copyright header with SPDX identifier at the
**top of the file**. For ``.cpp``, ``.hpp``, ``.h``, and ``.c`` files the exact format is:

.. code-block:: cpp

    /********************************************************************************
     * Copyright (c) <year> <author>
     *
     * This program and the accompanying materials are made available under the
     * terms of the Apache License Version 2.0 which is available at
     * https://www.apache.org/licenses/LICENSE-2.0
     *
     * SPDX-License-Identifier: Apache-2.0
     ********************************************************************************/

- The year must be a 4-digit year (a range such as ``2024-2026`` is also acceptable).
- The author/company name must be present.
- The SPDX identifier ``Apache-2.0`` must be included.
- Old-style single-line headers (e.g. ``// Copyright 2024 Accenture.``) are **no longer valid**.

The ``cr_checker.py`` tool can automatically verify and fix copyright headers.
See :ref:`automatic_formatting` → *Copyright Header Check* for details.
