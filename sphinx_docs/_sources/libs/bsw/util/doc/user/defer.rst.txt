..
   *******************************************************************************
   Copyright (c) 2024 Accenture

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

.. _util_defer:

`util::defer`
=============

``util::defer``'s functionality allows deferring a function call until the visibility scope of the
enclosing block is exited. This functionality is based on the `RAII idiom`, meaning the specified
callable will be executed in the `destructor` of the `defer object`.

.. code-block:: cpp

    void testFunc()
    {
        int val = 0;
        {
            auto _ = util::defer::defer([&val]() { val++; });
            assert(0 == val);
        }
        assert(1 == val);
    }
