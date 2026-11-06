..
   *******************************************************************************
   Copyright (c) 2024 Accenture

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

Bitmask operations
==================

Overview
--------

Template class ``Mask`` provides the basic functionality for bitmask operations, such as adding and removing bit,
comparing masks for equality, overlap and similar tasks.

Example
-------

.. code-block:: cpp

    Mask<uint8_t, uint32_t> mask1, mask2; // uint8_t is type of the position, uint32_t is type of underlying integer
    mask1.add(1); // add bit at position 1
    mask2.add(2); // add bit at position 2
    bool isOverlapping = mask1.overlaps(mask2); // returns false
