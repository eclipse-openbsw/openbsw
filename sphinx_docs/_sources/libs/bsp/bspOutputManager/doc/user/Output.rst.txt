..
   *******************************************************************************
   Copyright (c) 2024 Accenture

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

..  _bspOutputManager_Output:

Output
======

``Output`` provides methods for handling output operations, including initialization and shutdown.

The `output` class initializes output pins based on ``OutputConfiguration`` and it provides an
application interface for reading the status of the output pins.

It supports the following functionalities:
    * Set IO Channel
    * Get IO Channel
    * Invert IO Channel
    * Release IO Channel